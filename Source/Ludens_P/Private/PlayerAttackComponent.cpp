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

	if (!MeleeAttackHandler)
	{
		MeleeAttackHandler = NewObject<UMeleeAttackHandler>(this);
		MeleeAttackHandler->OwnerCharacter = Cast<ACharacter>(GetOwner());
	}
	
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
	if (!MeleeAttackHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("MeleeAttackHandler is nullptr!"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("TryMeleeAttack Called!"));
	AttackDamage = 99999;
	if (!GetOwner()->HasAuthority())
	{
		// 클라이언트라면 서버 RPC 호출
		Server_TryMeleeAttack();
		return;
	}
	// 서버라면 실제 공격 처리
	MeleeAttackHandler->HandleMeleeAttack(AttackDamage);
}

void UPlayerAttackComponent::Server_TryMeleeAttack()
{
	UE_LOG(LogTemp, Display, TEXT("TryMeleeAttack Called!"));
	AttackDamage = 99999;
	MeleeAttackHandler->HandleMeleeAttack(AttackDamage);
}

// Called every frame
void UPlayerAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

