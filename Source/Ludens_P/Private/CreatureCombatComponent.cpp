#include "CreatureCombatComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCreatureCombatComponent::UCreatureCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UCreatureCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}



void UCreatureCombatComponent::TakeDamage(float Amount)
{
	if (bIsDead) return;

	CurrentHP -= Amount;
	if (CurrentHP <= 0.f)
	{
		Die();
	}
}

void UCreatureCombatComponent::Die()
{
	bIsDead = true;
	UE_LOG(LogTemp, Warning, TEXT("Enemy Died"));
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		OwnerChar->DetachFromControllerPendingDestroy();
		OwnerChar->SetLifeSpan(5.f);
		OwnerChar->GetMesh()->SetSimulatePhysics(true);
	}
}

float UCreatureCombatComponent::GetHealthPercent() const
{
	return (MaxHP > 0.f) ? CurrentHP / MaxHP : 0.f;
}
