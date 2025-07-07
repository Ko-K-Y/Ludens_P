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
	
public:
	void HandleMeleeAttack();
};
