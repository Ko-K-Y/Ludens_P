// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"

#include "PlayerStateComponent.generated.h"

// 플레이어의 상태를 체크하는 컴포넌트
// 쉴드, 체력, 이동속도, 피격 상태, 생존 상태 확인

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UPlayerStateComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY()
	class ACharacter* Character;
	
public:	
	// Sets default values for this component's properties
	UPlayerStateComponent();

	// 플레이어 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Player", Replicated)
	float MaxHP = 100.0f;
	UPROPERTY(VisibleAnywhere, Category = "Player", Replicated)
	float CurrentHP;

	// 플레이어 쉴드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", Replicated)
	float MaxShield = 100.0f;
	UPROPERTY(VisibleAnywhere, Category = "Player", Replicated)
	float CurrentShield;

	// 플레이어 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", ReplicatedUsing=OnRep_MoveSpeed)
	float MoveSpeed;
	float CalculateMoveSpeed; // 능력 강화시 계산용
	float KnockedMoveSpeed = 100.0f;
	float DeadMoveSpeed = 0.f;
	
	// 플레이어가 공격 당한 상태인지 확인, 공격 당하면 일정 시간 동안 무적 상태
	UPROPERTY(VisibleAnywhere, Category = "Player", ReplicatedUsing=OnRep_IsAttacked)
	bool IsAttacked = false;
	FTimerHandle InvincibilityTimerHandle;

	//플레이어의 생존 여부
	UPROPERTY(VisibleAnywhere, Category = "Player", ReplicatedUsing=OnRep_Dead)
	bool IsDead = false;

	//플레이어의 생존 여부
	UPROPERTY(VisibleAnywhere, Category = "Player", ReplicatedUsing=OnRep_Knocked)
	bool IsKnocked = false;

	UPROPERTY(VisibleAnywhere, Category = "Player", Replicated)
	bool bCanRegenShield = false;
	
	void Knocked();
	UFUNCTION(Server, Reliable)
	void Server_Knocked();
	void Dead();
	void TakeDamage(float Amount);
	void EnableShieldRegen();
	void RegenShieldHandle();
protected:
	FTimerHandle bCanRegenShieldTimer;
	FTimerHandle RegenShieldTimer;
public:
	void ResetInvincibility(); // 무적 시간 초기화 함수
	UFUNCTION()
	void OnRep_IsAttacked(); // 피격 당한 상태(무적 시간)일 때 UI 또는 이펙트를 적용하는 함수
	UFUNCTION()
	void OnRep_Dead(); // 죽은 상태일 때 UI 또는 이펙트를 적용하는 함수
	UFUNCTION()
	void OnRep_Knocked();
	UFUNCTION()
	void OnRep_MoveSpeed();

	void UpdateMoveSpeed();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	FTimerHandle KnockedTimer; // 기절 한 뒤 죽을 때까지 작동하는 타이머
};
