// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStateComponent.h"

#include "SWarningOrErrorBox.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "DSP/Delay.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UPlayerStateComponent::UPlayerStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); // 컴포넌트 복제를 활성화합니다.
}


// Called when the game starts
void UPlayerStateComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		CurrentHP = MaxHP;
		CurrentShield = MaxShield;
		MoveSpeed = 600.0f; // 기본 이동 속도 설정
		CalculateMoveSpeed = MoveSpeed;
	}
	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		// 이 컴포넌트가 부착된 캐릭터의 초기 이동 속도를 설정합니다.
		// OnRep_MoveSpeed가 초기값에 대해 호출될 수 있으므로 클라이언트에서도 필요합니다.
		Character->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
}

void UPlayerStateComponent::TakeDamage(float Amount)
{
	if (IsDead || IsKnocked) return;
	if (IsAttacked) return; // 공격 받은 후 무적 상태
	if (CurrentHP <= 0.f) Knocked();

	bCanRegenShield = false;
	GetWorld()->GetTimerManager().ClearTimer(RegenShieldTimer);
	GetWorld()->GetTimerManager().ClearTimer(bCanRegenShieldTimer);

	// 쉴드가 남아 있을 경우 쉴드가 먼저 데미지를 받음.
	if (CurrentShield > 0)
	{
		CurrentShield = FMath::Max(0.f, CurrentShield - Amount);
		UE_LOG(LogTemp, Warning, TEXT("Shield: %f"), CurrentShield);

	}
	else
	{
		CurrentHP = FMath::Max(0.f, CurrentHP - Amount);
		UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHP);
	}

	GetWorld()->GetTimerManager().SetTimer(bCanRegenShieldTimer, this, &UPlayerStateComponent::EnableShieldRegen, 5.0f, false);
	
	if (CurrentHP <= 0.f) Knocked();
	else
	{
		// 공격 당한 상태로 설정하고 무적 타이머 시작
		IsAttacked = true;

		// 2초 후 무적 상태 해제
		const float InvincibilityDuration = 1.0f;
		GetWorld()->GetTimerManager().SetTimer(InvincibilityTimerHandle, this, &UPlayerStateComponent::ResetInvincibility, InvincibilityDuration, false);
	}
}

void UPlayerStateComponent::EnableShieldRegen()
{
	bCanRegenShield = true;
	GetWorld()->GetTimerManager().SetTimer(RegenShieldTimer, this, &UPlayerStateComponent::RegenShieldHandle, 0.5f, true);
}

void UPlayerStateComponent::RegenShieldHandle()
{
	if (!bCanRegenShield || CurrentShield >= MaxShield)
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenShieldTimer);
		return;
	}
	CurrentShield += 1.f;
	UE_LOG(LogTemp, Log, TEXT("Shield Regen: %f"), CurrentShield);
}

void UPlayerStateComponent::Knocked()
{
	// Knocked 상태 변경은 서버에서만 실행
	if (!GetOwner()->HasAuthority()) Server_Knocked();
	
	UE_LOG(LogTemp, Error, TEXT("Player Knocked!"));
	
	IsKnocked = true;
	MoveSpeed = KnockedMoveSpeed;
	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	// OnRep 함수를 수동으로 호출하여 서버 자신에게도 로직을 적용합니다.
	OnRep_Knocked();
	
	// 5초 뒤 Dead 함수를 호출하는 타이머를 설정
	GetWorld()->GetTimerManager().SetTimer(KnockedTimer, this, &UPlayerStateComponent::Dead, 15.0f, false);
}

void UPlayerStateComponent::Server_Knocked_Implementation()
{
	UE_LOG(LogTemp, Error, TEXT("Client Player Knocked!"));
	Knocked();
}

void UPlayerStateComponent::Dead()
{
	// Dead 상태 변경은 서버에서만 실행
	if (!GetOwner()->HasAuthority()) return;
	
	UE_LOG(LogTemp, Error, TEXT("Player Dead!"));
	IsKnocked = false;
	IsDead = true;
	MoveSpeed = DeadMoveSpeed;
	
	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	OnRep_Dead();
	
}

void UPlayerStateComponent::ResetInvincibility() 
{
	IsAttacked = false;
}

void UPlayerStateComponent::OnRep_IsAttacked()
{
	// 클라이언트에서 피격 상태 변경 시 처리할 로직 (UI, 이펙트 등)
}

void UPlayerStateComponent::OnRep_Dead()
{
	// 클라이언트에서 죽은 상태 변경 시 처리할 로직 (UI, 이펙트 등)
	UE_LOG(LogTemp, Warning, TEXT("Client OnRep_Dead called."));
}

void UPlayerStateComponent::OnRep_Knocked()
{
	// 클라이언트에서 기절 상태 변경 시 처리할 로직 (UI, 이펙트 등)
	UE_LOG(LogTemp, Warning, TEXT("Client OnRep_Knocked called."));
}

void UPlayerStateComponent::OnRep_MoveSpeed()
{
	// 클라이언트에서 MoveSpeed 값이 복제되었을 때 호출됨.
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client OnRep_MoveSpeed called. New Speed: %f"), MoveSpeed);
		Character->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
}

void UPlayerStateComponent::UpdateMoveSpeed()
{
	// 반드시 서버에서 실행
	if (GetOwner()->HasAuthority())
	{
		MoveSpeed = CalculateMoveSpeed;

		// 만약 클라이언트가 호스트(리스닝 서버)인 경우, OnRep 함수가 자동으로 호출되지 않으므로 수동으로 호출해줍니다.
		OnRep_MoveSpeed();
	}
}

void UPlayerStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UPlayerStateComponent, MaxHP);
	DOREPLIFETIME(UPlayerStateComponent, CurrentHP);
	DOREPLIFETIME(UPlayerStateComponent, MaxShield);
	DOREPLIFETIME(UPlayerStateComponent, CurrentShield);
	DOREPLIFETIME(UPlayerStateComponent, IsAttacked);
	DOREPLIFETIME(UPlayerStateComponent, IsKnocked);
	DOREPLIFETIME(UPlayerStateComponent, IsDead);
	DOREPLIFETIME(UPlayerStateComponent, MoveSpeed);
}

