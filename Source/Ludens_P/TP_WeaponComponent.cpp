// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"

//#include "EnemyBase.h"
//#include "EnemyPoolManager.h"
#include "Ludens_PCharacter.h"
#include "Ludens_PProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "JellooComponent.h"
#include "Net/UnrealNetwork.h"
#include "Projects.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

// Sets default values for this component's properties
// 무기 종류와 Fire 메서드, Jelloo 흡수 등 무기와 관련된 메서드들
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(50.0f, 100.0f, 0.0f);
}
void UTP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Character)
	{
		Character = Cast<ALudens_PCharacter>(GetOwner());
		if (!Character)
		{
			UE_LOG(LogTemp, Error, TEXT("WeaponComponent could not find owning character!"));
		}
	}
}

void UTP_WeaponComponent::Fire()
{
	// 위치 및 방향
	FVector CameraLocation = Character->FirstPersonCameraComponent->GetComponentLocation();
	FRotator SpawnRotation = Character->GetActorRotation();
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		SpawnRotation = PC->PlayerCameraManager->GetCameraRotation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("❗ GetController() is null, fallback to actor rotation"));
	}
	// 카메라 앞쪽에서 발사
	constexpr float Distance = 10.0f;
	FVector FireDirection = SpawnRotation.Vector();
	FVector SpawnLocation = CameraLocation + FireDirection * Distance;

	if (Character->HasAuthority())
	{
		HandleFire(SpawnLocation, SpawnRotation);
	}
	else
	{
		ServerFire(SpawnLocation, SpawnRotation);
	}
}
void UTP_WeaponComponent::ServerFire_Implementation(FVector_NetQuantize SpawnLocation, FRotator SpawnRotation)
{
	// 클라이언트가 요청 → 서버에서 처리
	HandleFire(SpawnLocation, SpawnRotation);
}
void UTP_WeaponComponent::HandleFire(const FVector& SpawnLocation, const FRotator& SpawnRotation) //서버에서 쓰는 Fire (얘가 진짜 Projectile을 쏘는거임)
{
	if (!ProjectileClass) //프로젝타일 null값 방지
	{
		UE_LOG(LogTemp, Error, TEXT("❌ ProjectileClass is null"));
		return;
	}

	if (!Character) //캐릭터 null값 방지
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Character is null in HandleFire"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	ALudens_PProjectile* Projectile = GetWorld()->SpawnActor<ALudens_PProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams); // (스폰 위치, 방향, 액터가 게임 월드가 스폰될 때 디테알한 부분을 조정 가능.)
	if (!Projectile)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Projectile spawn failed"));
		return;
	}

	// 🔊 서버에서도 사운드/애니메이션 재생은 Multicast 또는 클라 전용으로 처리 필요

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	if (FireAnimation)
	{
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::ServerAbsorb_Implementation()
{
	HandleAbsorb();
}

void UTP_WeaponComponent::HandleAbsorb()
{
	// 반드시 서버에서만 로직 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		ServerAbsorb();
		return;
	}

	ALudens_PCharacter* Ludens_PCharacter = Cast<ALudens_PCharacter>(Character);
	if (!Ludens_PCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is not ALudens_PCharacter!"));
		return;
	}
		
	// 화면 중심에서 월드 방향 구하기
	FVector WorldLocation = Ludens_PCharacter->FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRotation = Character->GetActorRotation();
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	}

	// 트레이스 시작/끝 위치 계산
	FVector TraceDirection = CameraRotation.Vector();
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = TraceStart + (TraceDirection * AbsorbRange);

	// 라인 트레이스
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// 라인 트레이스를 하여 무언가에 맞았는지를 나타냄
	bool bHit = Character->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params);

	// 6. CreatureCombatComponent가 있으면 데미지 적용
	if (bHit && Hit.GetActor())
	{
		// 맞은 액터가 JellooComp를 가지고 있는지 검사;
		if (UJellooComponent* JellooComp = Hit.GetActor()->FindComponentByClass<UJellooComponent>())
		{
			TargetJelloo = JellooComp;
			// 타이머가 이미 작동 중인지 먼저 확인
			if (!GetWorld()->GetTimerManager().IsTimerActive(AbsorbDelayTimer))
			{
				GetWorld()->GetTimerManager().SetTimer(AbsorbDelayTimer, this, &UTP_WeaponComponent::PerformAbsorb, AbsorbDelay, false);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("<UNK> Absorb failed"));
		StopPerformAbsorb();
		return;
	}
}

void UTP_WeaponComponent::PerformAbsorb()
{
	if (!TargetJelloo)
	{
		// 타겟이 없으면 타이머를 중지하거나 무시
		GetWorld()->GetTimerManager().ClearTimer(AbsorbDelayTimer);
		return;
	}
	TargetJelloo->JellooTakeDamage(AbsorbAmount);
	Character->SavedAmmo += AbsorbAmount;
}

void UTP_WeaponComponent::StopPerformAbsorb()
{
	UE_LOG(LogTemp, Error, TEXT("StopPerformAbsorb"));
	TargetJelloo = nullptr;
	GetWorld()->GetTimerManager().ClearTimer(AbsorbDelayTimer);
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) //필요 없을거같은데
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}

void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTP_WeaponComponent, AbsorbAmount);
	
}