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
	if (PlayerAttackComponent && WeaponComponent)
	{
		PlayerAttackComponent->WeaponAttackHandler->WeaponComp = WeaponComponent;
	}
	
	if (!DashAction)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("DashAction is null!"));
	}
	else if (!MeleeAttackAction)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("MeleeAttackAction is null!"));
	}
	else if (!PlayerAttackComponent)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("PlayerAttackComponent is null!"));
	}
	else if (!PlayerStateComponent)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("PlayerStateComponent is null!"));
	}
	
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
		EnhancedInputComponent->BindAction(MeleeAttackAction, ETriggerEvent::Started, this, &ALudens_PCharacter::MeleeAttack);

		// TestAttack -> P
		EnhancedInputComponent->BindAction(TestAttackAction, ETriggerEvent::Started, this, &ALudens_PCharacter::TestAttack);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Reload);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ALudens_PCharacter::Fire);
	}
}


void ALudens_PCharacter::Move(const FInputActionValue& Value)
{
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
		UE_LOG(LogTemplateCharacter, Warning, TEXT("TestAttack!"));
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
	UE_LOG(LogTemp, Warning, TEXT("Server_Reload_Implementation called (Role: %d)"), GetLocalRole());
	HandleReload();
}

void ALudens_PCharacter::Reload(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Reload() called (Role: %d)"), GetLocalRole());

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
	UE_LOG(LogTemp, Warning, TEXT("HandleReload() called (Role: %d)"), GetLocalRole());
	if (CurrentAmmo != MaxAmmo)
	{
		if (SavedAmmo <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Case1: Saved Ammo is 0"));
			return;
		}
		else if (SavedAmmo - (MaxAmmo - CurrentAmmo) <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Case2: Left Ammo Reloaded"));
			CurrentAmmo += SavedAmmo;
			SavedAmmo = 0;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Case3: Reload Complete"));
			SavedAmmo -= (MaxAmmo-CurrentAmmo);
			CurrentAmmo = MaxAmmo;
		}
		UE_LOG(LogTemp, Warning, TEXT("Current Ammo %d"), CurrentAmmo);
		UE_LOG(LogTemp, Warning, TEXT("Saved Ammo %d"), SavedAmmo);
	}
}

void ALudens_PCharacter::OnRep_SavedAmmo()
{
	// 젤루 흡수 or 재장전 시 변경되는 UI, 사운드 등
	UE_LOG(LogTemp, Warning, TEXT("OnRep_Saved Ammo %d"), SavedAmmo);

}

void ALudens_PCharacter::OnRep_CurrentAmmo()
{
	// 재장전 시 변경되는 UI, 사운드 등
	UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentAmmo : %d"), CurrentAmmo);

}

int16 ALudens_PCharacter::GetCurrentAmmo() const
{
	return CurrentAmmo;
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
