#pragma once

#include "CoreMinimal.h"
#include "CreatureCombatComponent.h"
#include "WalkerAIComponent.h"
#include "TP_WeaponComponent.h"
#include "StealthComponent.h"
#include "GameFramework/Character.h"
#include "EnemyColorEnum.h"
#include "EnemyBase.generated.h"

UCLASS()
class LUDENS_P_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// === 전투 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	UCreatureCombatComponent* Combat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStealthComponent* StealthComponent;
	
	virtual void SetActive(bool bNewActive);
	void PostNetInit();
	void OnPostReplicationInit();
	bool IsActive() const;

	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetActive(bool bNewActive);

	//원거리 형 적들이 공격을 할 수 있게
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UTP_WeaponComponent* ShooterWeaponComponent;

	UFUNCTION(BlueprintCallable, Category = "Enemy Properties")
	void SetupEnemyForColor(EEnemyColor NewColor);

	// 특정 색상의 쉴드에 데미지를 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy Properties|Shields")
	void ApplyShieldDamage(EEnemyColor DamageColor, int32 DamageAmount);

	// 쉴드를 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy Properties|Shields")
	void InitializeShields();
	
protected:
	// Called on clients when EnemyColor is replicated.
	UFUNCTION()
	void OnRep_EnemyColor();

	// Applies the material based on the current EnemyColor.
	void UpdateMaterial();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Properties")
	TMap<EEnemyColor, UMaterialInstance*> EnemyMaterials;

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	int32 MaterialSlotIndex;

	// 이 적의 색상 조합 (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_EnemyColor)
	EEnemyColor EnemyColor;

	// 각 기본 색상별 최대 쉴드량을 정의 (블루프린트에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Properties|Shields")
	TMap<EEnemyColor, int32> DefaultShields;

	// 현재 적이 가진 쉴드들의 종류와 현재량 (런타임에 변경됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Properties|Shields")
	TMap<EEnemyColor, int32> Shields;
	
private:
	bool bActive = false;

};
