// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WeaponAttackHandler.generated.h"

/**
 * 
 */

UCLASS()
class LUDENS_P_API UWeaponAttackHandler : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	class UCreatureCombatComponent* CreatureCombatComp;
	
public:
	UPROPERTY()
	ACharacter* OwnerCharacter;

	void HandleWeaponAttack(float damage);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float WeaponRange = 200.0f;
	
protected:
	virtual void BeginPlay();
};
