// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAttackHandler.h"

#include "CreatureCombatComponent.h"
#include "GameFramework/Character.h"
#include "../TP_WeaponComponent.h"
#include "Ludens_P/Ludens_PCharacter.h"


void UWeaponAttackHandler::HandleWeaponAttack(float damage)
{
	UE_LOG(LogTemp, Display, TEXT("HandleWeaponAttack!"));

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is nullptr!"));
		return;
	}
	// 무기 공격 로직 -> 공격 로직, 이펙트, 사운드 등등
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponComp is nullptr in HandleWeaponAttack!"));
		return;
	}
	WeaponComp->Fire();
}




