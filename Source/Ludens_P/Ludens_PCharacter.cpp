// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ludens_PCharacter.h"
#include "Ludens_PProjectile.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PlayerAttackComponent.h"
#include "PlayerStateComponent.h"
#include "TP_WeaponComponent.h"
#include "WeaponAttackHandler.h"
#include "CreatureCombatComponent.h"
#include "JellooComponent.h"
#include "ReviveComponent.h"

#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ALudens_PCharacter

ALudens_PCharacter::ALudens_PCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	JumpCount = 0; // Default 점프 수 설정
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 1.0f;
	OriginalGroundFriction = 8.0f;
	OriginalBrakingDeceleration = 2048.0f;

	// 네트워크 업데이트 주기 향상
	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 50.0f;

	// 이동 컴포넌트 복제 설정
	GetCharacterMovement()->SetIsReplicated(true);

	// 초기 탄알 수 설정
	CurrentAmmo = MaxAmmo;
}

void ALudens_PCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	CurrentDashCount = MaxDashCount; // 게임 시작 시 최대 대쉬 충전

	// 로컬 플레이어 컨트롤러 확인
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// 기본 입력 매핑 컨텍스트 추가
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//컴포넌트 할당
	PlayerAttackComponent = FindComponentByClass<UPlayerAttackComponent>();
	PlayerStateComponent = FindComponentByClass<UPlayerStateComponent>();
	WeaponComponent = FindComponentByClass<UTP_WeaponComponent>();
	ReviveComponent =  FindComponentByClass<UReviveComponent>();
	
	if (PlayerAttackComponent && WeaponComponent)
	{
		PlayerAttackComponent->WeaponAttackHandler->WeaponComp = WeaponComponent;
	}
	
	if (!DashAction) UE_LOG(LogTemplateCharacter, Error, TEXT("DashAction is null!"))
	
	else if (!MeleeAttackAction) UE_LOG(LogTemplateCharacter, Error, TEXT("MeleeAttackAction is null!"))
	
	else if (!PlayerAttackComponent) UE_LOG(LogTemplateCharacter, Error, TEXT("PlayerAttackComponent is null!"))
	
	else if (!PlayerStateComponent) UE_LOG(LogTemplateCharacter, Error, TEXT("PlayerStateComponent is null!"))
	
	else if (!ReviveComponent) UE_LOG(LogTemplateCharacter, Error, TEXT("ReviveComponent is null!"));
	
	
}

void ALudens_PCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALudens_PCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALudens_PCharacter::Look);

		// Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Dash);

		// MeleeAttack
		EnhancedInputComponent->BindAction(MeleeAttackAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Interact);

		// TestAttack -> P
		EnhancedInputComponent->BindAction(TestAttackAction, ETriggerEvent::Started, this, &ALudens_PCharacter::TestAttack);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Reload);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Fire);

		// Revive
		EnhancedInputComponent->BindAction(ReviveAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Interact);

		// Absorb
		EnhancedInputComponent->BindAction(AbsorbAction, ETriggerEvent::Ongoing, this, &ALudens_PCharacter::Interact);
		EnhancedInputComponent->BindAction(AbsorbAction, ETriggerEvent::Completed, this, &ALudens_PCharacter::AbsorbComplete);
	}
}


void ALudens_PCharacter::Move(const FInputActionValue& Value)
{
	if (ReviveComponent && ReviveComponent->IsReviving())
	{
		ReviveComponent->CancelRevive(); // ← ReviveTimer 해제 + KnockedTimer 재개
	}
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALudens_PCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ALudens_PCharacter::TestAttack(const FInputActionValue& Value)
{
	if (PlayerStateComponent)
	{
		PlayerStateComponent->TakeDamage(100.0f);
	}
}

void ALudens_PCharacter::Jump()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_Jump();
		return;
	}
	
	if (ReviveComponent && ReviveComponent->IsReviving())
	{
		ReviveComponent->CancelRevive(); // ← ReviveTimer 해제 + KnockedTimer 재개
	}
	
	if (JumpCount < MaxJumpCount)
	{
		Super::Jump();
		JumpCount++;
		if (JumpCount <= MaxJumpCount)
		{
			LaunchCharacter(FVector(0,0,600), false, true);
		}
	}
}

// 서버 전용 점프 처리
void ALudens_PCharacter::Server_Jump_Implementation()
{
	Jump();
}


void ALudens_PCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (GetLocalRole() == ROLE_Authority)
	{
		JumpCount = 0; // 서버에서 JumpCount 리셋
	}
}

void ALudens_PCharacter::Dash(const FInputActionValue& Value)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_Dash();
		return;
	}
	
	if (ReviveComponent && ReviveComponent->IsReviving())
	{
		ReviveComponent->CancelRevive(); // ← ReviveTimer 해제 + KnockedTimer 재개
	}
	
	if (bCanDash && CurrentDashCount > 0)
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (!MoveComp) return;

		GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle); // 기존에 돌아가던 타이머 취소
	
		// 1. 원본 값 백업
		OriginalGroundFriction = MoveComp->GroundFriction;
		OriginalBrakingDeceleration = MoveComp->BrakingDecelerationWalking;

		// 2. 대시 중 마찰력 제거
		MoveComp->GroundFriction = 1.0f;
		MoveComp->BrakingDecelerationWalking = 1.0f;

		// 3. 대시 방향 계산
		FVector DashDirection = MoveComp->Velocity;
		DashDirection.Z = 0;
		DashDirection.Normalize();
		if (DashDirection.IsNearlyZero())
		{ 
			DashDirection = GetActorForwardVector();
		}

		if (CurrentDashCount <= 0) return;

		// 서버에서 강제 실행
		LaunchCharacter(DashDirection * DashSpeed, true, true);
		
		CurrentDashCount--;
		bCanDash = false;
	
		// 5. 0.2초 후 원래 값 복원 (대시 지속시간에 맞게 조절)
		GetWorld()->GetTimerManager().SetTimer(
			DashPhysicsTimerHandle,
			[this]()
			{
				if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
				{
					MoveComp->GroundFriction = OriginalGroundFriction;
					MoveComp->BrakingDecelerationWalking = OriginalBrakingDeceleration;
				}
			},
			0.2f,
			false
		);

		// 1초 후 다음 대쉬 가능
		GetWorld()->GetTimerManager().SetTimer(
			DashCooldownTimerHandle,
			[this]() { bCanDash = true; },
			DashCooldown,
			false
		);

		//3초마다 대쉬 충전
		if (!GetWorld()->GetTimerManager().IsTimerActive(DashRechargeTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(
				DashRechargeTimerHandle,
				this,
				&ALudens_PCharacter::RechargeDash,
				DashRechargeTime,
				true
			);
		}
	}
}

void ALudens_PCharacter::Server_Dash_Implementation()
{
	Dash(FInputActionValue()); // 실제 대쉬 실행
}

void ALudens_PCharacter::RechargeDash()
{
	if (GetLocalRole() == ROLE_Authority && CurrentDashCount < MaxDashCount)
	{
		CurrentDashCount++;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(DashRechargeTimerHandle);
	}
}

void ALudens_PCharacter::ResetMovementParams() const
{
	// 대쉬 끝난 뒤 마찰력 초기화
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->GroundFriction = OriginalGroundFriction;
		MoveComp->BrakingDecelerationWalking = OriginalBrakingDeceleration;
	}
}

void ALudens_PCharacter::Interact(const FInputActionValue& Value) // 앞에 있는 대상이 무엇인지 판별해주는 메서드
{
	if (ReviveComponent && ReviveComponent->IsReviving())
	{
		ReviveComponent->CancelRevive(); // ← ReviveTimer 해제 + KnockedTimer 재개
	}
	// 라인 트레이스를 통해 앞에 있는 대상이 무엇인지 판별
	// 화면 중심에서 월드 방향 구하기
	FVector WorldLocation = FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRotation = GetActorRotation();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	}
	else UE_LOG(LogTemp, Warning, TEXT("❗ GetController() is null, fallback to actor rotation"));

	FVector TraceDirection = CameraRotation.Vector();
	// 트레이스 시작/끝 위치 계산
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = TraceStart + (TraceDirection * 150.f);

	// 라인 트레이스
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// 라인트레이스 선 나타내기
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 2.0f);
	
	// 라인 트레이스를 하여 무언가에 맞았는지를 나타냄
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params);
	
	if (bHit && Hit.GetActor())
	{
		// 맞은 액터가 어떤 컴포넌트를 가지고 있는지 검사
		if (Hit.GetActor()->FindComponentByClass<UCreatureCombatComponent>())
		{
			MeleeAttack(Value);
		}
		else if (Hit.GetActor()->FindComponentByClass<UPlayerStateComponent>())
		{
			Revive(Value);
		}
		else if (Hit.GetActor()->FindComponentByClass<UJellooComponent>())
		{
			Absorb(Value);
		}
	}
	else if (!Hit.GetActor())
	{
		UE_LOG(LogTemp, Error, TEXT("Hit.GetActor() is null!"));
		return;
	}
}

void ALudens_PCharacter::MeleeAttack(const FInputActionValue& Value)
{
	//근접 공격 로직 호출
	if (PlayerAttackComponent)
	{
		PlayerAttackComponent->TryMeleeAttack();
	}
}

void ALudens_PCharacter::Server_Fire_Implementation(const FInputActionValue& Value)
{
	Fire(Value); // 서버에서 발사 처리
}

void ALudens_PCharacter::Fire(const FInputActionValue& Value)
{
	// 무기 공격 로직 호출
	if (GetLocalRole() < ROLE_Authority)
	{
		// 클라: 서버에 발사 요청 RPC 호출
		Server_Fire(Value);
		return;
	}
	if (CurrentAmmo > 0)
	{
		// 서버: 실제 발사 처리
		WeaponComponent->Fire();
		CurrentAmmo --;
	}
}

void ALudens_PCharacter::Server_Reload_Implementation()
{
	HandleReload();
}

void ALudens_PCharacter::Reload(const FInputActionValue& Value)
{
	if (ReviveComponent && ReviveComponent->IsReviving())
	{
		ReviveComponent->CancelRevive(); // ← ReviveTimer 해제 + KnockedTimer 재개
	}
	

	if (GetLocalRole() < ROLE_Authority)
	{
		// 클라이언트 플레이어
		Server_Reload();
		return;
	}
	else HandleReload(); // 서버 플레이어
}

void ALudens_PCharacter::HandleReload()
{
	if (CurrentAmmo != MaxAmmo)
	{
		if (SavedAmmo <= 0)
		{
			return;
		}
		else if (SavedAmmo - (MaxAmmo - CurrentAmmo) <= 0)
		{
			CurrentAmmo += SavedAmmo;
			SavedAmmo = 0;
		}
		else
		{
			SavedAmmo -= (MaxAmmo-CurrentAmmo);
			CurrentAmmo = MaxAmmo;
		}
	}
}

void ALudens_PCharacter::OnRep_SavedAmmo()
{
	// 젤루 흡수 or 재장전 시 변경되는 UI, 사운드 등
}

void ALudens_PCharacter::OnRep_CurrentAmmo()
{
	// 재장전 시 변경되는 UI, 사운드 등
}

int16 ALudens_PCharacter::GetCurrentAmmo() const // PlayerAttackComponent에서 현재 탄알 수 확인용
{
	return CurrentAmmo;
}

void ALudens_PCharacter::Server_Revive_Implementation()
{
	// 서버에서 소생 처리
	ReviveComponent->HandleRevive();
}

void ALudens_PCharacter::Revive(const FInputActionValue& Value)
{
	// 만약 클라이면 -> 서버에게 소생 요청
	if (GetLocalRole() < ROLE_Authority) Server_Revive();
	ReviveComponent->HandleRevive();
}

void ALudens_PCharacter::Absorb(const FInputActionValue& Value)
{
	if (GetLocalRole() < ROLE_Authority) Server_Absorb(Value);
	WeaponComponent->HandleAbsorb();
}

void ALudens_PCharacter::Server_Absorb_Implementation(const FInputActionValue& Value)
{
	Absorb(Value);
}

void ALudens_PCharacter::AbsorbComplete(const FInputActionValue& Value)
{
	
	if (GetLocalRole() < ROLE_Authority) Server_AbsorbComplete(Value);
	GetWorldTimerManager().SetTimer(
	   AbsorbCompleteTimerHandle,
	   FTimerDelegate::CreateUObject(WeaponComponent, &UTP_WeaponComponent::StopPerformAbsorb),
	   0.3f, 
	   false
   );
	UE_LOG(LogTemp, Log, TEXT("AbsorbComplete"));
}

void ALudens_PCharacter::Server_AbsorbComplete_Implementation(const FInputActionValue& Value)
{
	AbsorbComplete(Value);
}

void ALudens_PCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALudens_PCharacter, JumpCount);
	DOREPLIFETIME(ALudens_PCharacter, CurrentDashCount);
	DOREPLIFETIME(ALudens_PCharacter, bCanDash);
	DOREPLIFETIME(ALudens_PCharacter, SavedAmmo);
	DOREPLIFETIME(ALudens_PCharacter, CurrentAmmo);
}