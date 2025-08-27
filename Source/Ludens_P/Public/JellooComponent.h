// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "JellooComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UJellooComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJellooComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Getter 함수
	int16 GetCurrentJellooHP() const { return CurrentJellooHP; }

	// 루트로 사용할 메쉬 컴포넌트 (물리, 중력 적용용)
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	
protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Jelloo")
	int16 MaxJellooHP = 30;
	UPROPERTY(EditAnywhere, Category = "Jelloo", ReplicatedUsing = OnRep_JellooHP)
	int16 CurrentJellooHP;
	
	
public:
	void JellooTakeDamage(float Amount);
	
protected:
	void DestroyJelloo();
	UFUNCTION(Server, Reliable)
	void Server_DestroyJelloo();
	UFUNCTION()
	void OnRep_JellooHP();
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool bIsDead = false;
};
