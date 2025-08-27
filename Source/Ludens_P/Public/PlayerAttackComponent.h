// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "PlayerAttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UPlayerAttackComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY()
	class UWeaponAttackHandler* WeaponAttackHandler; // 무기 공격 클래스를 선언
private:
	UPROPERTY()
	class UMeleeAttackHandler* MeleeAttackHandler; // 근접 공격 클래스를 선언
	UPROPERTY()
	class ALudens_PCharacter* Character; // 캐릭터 클래스 선언
public:	
	// Sets default values for this component's properties
	UPlayerAttackComponent();

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackDamage; // 플레이어 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackCoolTime; // 플레이어 공격 쿨타임
	
	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float MeleeAttackDamage; // 플레이어 근접 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float MeleeAttackCoolTime; // 플레이어 근접 공격 쿨타임*/

	// 무기 공격 함수 호출
	UFUNCTION(Server, Reliable)
	void Server_TryWeaponAttack();
	UFUNCTION()
	void TryWeaponAttack();
	
	// 근접 공격 함수 호출
	UFUNCTION(Server, Reliable)
	void Server_TryMeleeAttack();
	void TryMeleeAttack();

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
