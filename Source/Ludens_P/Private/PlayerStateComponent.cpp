// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStateComponent.h"

// Sets default values for this component's properties
UPlayerStateComponent::UPlayerStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPlayerStateComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentShield = MaxShield;
}

void UPlayerStateComponent::Dead()
{
	if (CurrentHP <= 0)
	{
		IsDead = true;
	}
}

void UPlayerStateComponent::TakeDamage(float Amount)
{
	if (IsDead) return;
	if (IsAttacked) return; // 공격 받은 후 무적 상태
	if (CurrentHP <= 0.f) Dead();
	
	// 쉴드가 남아 있을 경우 쉴드가 먼저 데미지를 받음.
	if (CurrentShield > 0)
	{
		CurrentShield -= Amount;
	}
	else
	{
		CurrentHP -= Amount;
	}

	// 공격 당한 상태로 설정하고 무적 타이머 시작
	IsAttacked = true;

	// 2초 후 무적 상태 해제
	const float InvincibilityDuration = 2.0f;
	GetWorld()->GetTimerManager().SetTimer(InvincibilityTimerHandle, this, &UPlayerStateComponent::ResetInvincibility, InvincibilityDuration, false);
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
}

void UPlayerStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UPlayerStateComponent, MaxHP);
	DOREPLIFETIME(UPlayerStateComponent, CurrentHP);
	DOREPLIFETIME(UPlayerStateComponent, MaxShield);
	DOREPLIFETIME(UPlayerStateComponent, CurrentShield);
	DOREPLIFETIME(UPlayerStateComponent, IsAttacked);
	DOREPLIFETIME(UPlayerStateComponent, IsDead);
}

