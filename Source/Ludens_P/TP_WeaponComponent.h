// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Ludens_PProjectile.h"
#include "TP_WeaponComponent.generated.h"

class ALudens_PCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** The Character holding this weapon*/
	UPROPERTY()
	ALudens_PCharacter* Character;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	TSubclassOf<ALudens_PProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;
	
protected:
	UPROPERTY()
	class UJellooComponent* TargetJelloo = nullptr; // 현재 흡수하고 있는 젤루
	
public:
	/** Sets default values for this component's properties */
	UTP_WeaponComponent();
	void BeginPlay();
	
	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize SpawnLocation, FRotator SpawnRotation); // 클라이언트가 서버에 발사 요청할 RPC
	void HandleFire(const FVector& SpawnLocation, const FRotator& SpawnRotation); // 서버에서만 실행되는 진짜 발사 로직

	UFUNCTION(Server, Reliable)
	void ServerAbsorb(); // 클라이언트가 RPC 서버에 젤루 흡수 요청
	void HandleAbsorb(); // 서버에서 실행되는 젤루 흡수 로직
	void PerformAbsorb();
	void StopPerformAbsorb();
protected:
	UPROPERTY()
	float AbsorbRange = 200.f; // 젤루 흡수 사거리
	UPROPERTY()
	float AbsorbDelay = 0.3f; // 젤루 흡수 속도
	UPROPERTY(Replicated)
	int16 AbsorbAmount = 1; // 젤루 흡수 양 -> 나중에 이 수를 증가 시켜 젤루 흡수 속도 증가 같은 강화 요소로 쓸 수 있을 듯
	FTimerHandle AbsorbDelayTimer;
	bool bIsAbsorbing = false;
	
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};