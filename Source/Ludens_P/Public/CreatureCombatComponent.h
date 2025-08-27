#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CreatureCombatComponent.generated.h"

// CCC(CreatureCombatComponent)는 전투에 필요한 기본적인 스탯, 메서드
// 공격과 관련된 로직은 따로 분리 - 적의 타입마다 로직이 다르고 플레이어의 전투로직도 다름
// ---------------------------주요 메서드--------------------------
// TakeDamage - 피격 연산
// Die - 사망처리

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiedSignature);
// "체력이 변경되었다"는 신호를 보내기 위한 델리게이트 선언
// 파라미터: 현재 체력, 최대 체력
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDENS_P_API UCreatureCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCreatureCombatComponent();
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<AActor> DropItemClass; // 드랍되는 아이템은 액터로 고정.
	
	//Attack은 적의 타입, 플레이어에 따라 로직이 다양하므로 분리

	void TakeDamage(float Amount); //공격받는 메서드
	void Die(); //죽을 때
	UFUNCTION(Server, Reliable)
	void Server_Die();
	
	bool IsDead() const { return bIsDead; } //죽었는지 확인 (IsDead의 캡슐화를 위해 사용하는 Getter함수임)

	float GetHealthPercent() const;

	void InitStats(float InMaxHP);

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnHealthChangedSignature OnHealthChanged;

	// Getter 함수 만들어주기
	float GetCurrentHP() const { return CurrentHP; }
	float GetMaxHP() const { return MaxHP; }
	
protected:
	UPROPERTY(EditAnywhere, Replicated, Category="Combat")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentHP, Category="Combat")
	float CurrentHP;

	UFUNCTION()
	void OnRep_CurrentHP();

private:
	bool bIsDead = false;
	
};
