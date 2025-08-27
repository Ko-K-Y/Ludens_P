// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReviveComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UReviveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReviveComponent();

	UPROPERTY()
	class UPlayerStateComponent* PlayerStateComp;
	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	void HandleRevive();
	void HandleReviveComplete();
protected:
	UPROPERTY()
	UPlayerStateComponent* TargetPlayerState = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	FTimerHandle ReviveTimer;

public:
	bool IsReviving() const;
	void CancelRevive();
		
};
