// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MeleeAttackHandler.generated.h"

/**
 * 
 */

UCLASS()
class LUDENS_P_API UMeleeAttackHandler : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	class UCreatureCombatComponent* CreatureCombatComp;

public:
	UPROPERTY()
	ACharacter* OwnerCharacter;

	void HandleMeleeAttack(float damage);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float MeleeRange = 200.0f;

	
protected:
	virtual void BeginPlay();
};
