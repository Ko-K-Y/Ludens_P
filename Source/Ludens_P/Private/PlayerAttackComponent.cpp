// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackComponent.h"
#include "Net/UnrealNetwork.h"
#include "MeleeAttackHandler.h"
#include "WeaponAttackHandler.h"
#include "Ludens_P/Ludens_PCharacter.h"

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
	if (!WeaponAttackHandler)
	{
		WeaponAttackHandler = NewObject<UWeaponAttackHandler>(this);
		WeaponAttackHandler->OwnerCharacter = Cast<ACharacter>(GetOwner());

		ALudens_PCharacter* OwnerChar = Cast<ALudens_PCharacter>(GetOwner());
		if (OwnerChar)
		{
			WeaponAttackHandler->WeaponComp = OwnerChar->FindComponentByClass<UTP_WeaponComponent>();
		}
	}
	if (!Character)
	{
		Character = Cast<ALudens_PCharacter>(GetOwner());
	}
}

void UPlayerAttackComponent::Server_TryWeaponAttack_Implementation()
{
	AttackDamage = 30;
	WeaponAttackHandler->HandleWeaponAttack(AttackDamage);
}

void UPlayerAttackComponent::TryWeaponAttack()
{
	if (!WeaponAttackHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponAttackHandler is nullptr!"));
		return;
	}
	
	if (Character->GetCurrentAmmo() <= 0) return; // 남은 탄알이 0이면 공격 처리를 하지 않고 리턴
	
	AttackDamage = 30;
	// 무기 공격 함수 호출
	if (!GetOwner()->HasAuthority())
	{
		// 클라이언트라면 서버 RPC 호출
		Server_TryWeaponAttack();
		return;
	}
	// 서버라면 실제 공격 처리
	WeaponAttackHandler->HandleWeaponAttack(AttackDamage);
}

void UPlayerAttackComponent::Server_TryMeleeAttack_Implementation()
{
	AttackDamage = 99999;
	MeleeAttackHandler->HandleMeleeAttack(AttackDamage);
}

void UPlayerAttackComponent::TryMeleeAttack()
{
	if (!MeleeAttackHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("MeleeAttackHandler is nullptr!"));
		return;
	}

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

// Called every frame
void UPlayerAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

