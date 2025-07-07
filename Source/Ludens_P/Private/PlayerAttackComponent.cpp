// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackComponent.h"

#include "MeleeAttackHandler.h"

// Sets default values for this component's properties
UPlayerAttackComponent::UPlayerAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UPlayerAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UPlayerAttackComponent::TryAttack()
{
	AttackDamage = 30;
	// 무기 공격 함수 호출
}

void UPlayerAttackComponent::Server_TryAttack()
{
	
}

void UPlayerAttackComponent::TryMeleeAttack()
{
	AttackDamage = 99999;
	if (!GetOwner()->HasAuthority())
	{
		// 클라이언트라면 서버 RPC 호출
		Server_TryMeleeAttack();
		return;
	}
	// 서버라면 실제 공격 처리
	MeleeAttackHandler->HandleMeleeAttack();
}

void UPlayerAttackComponent::Server_TryMeleeAttack()
{
	MeleeAttackHandler->HandleMeleeAttack();
}

// Called every frame
void UPlayerAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

