#include "CreatureCombatComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCreatureCombatComponent::UCreatureCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
    SetIsReplicatedByDefault(true);
}

void UCreatureCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	
}

// ✨ GetLifetimeReplicatedProps 함수를 추가하여 CurrentHP를 복제 등록
void UCreatureCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCreatureCombatComponent, CurrentHP);
	DOREPLIFETIME(UCreatureCombatComponent, MaxHP);
}

void UCreatureCombatComponent::TakeDamage(float Amount)
{
	if (!GetOwner()->HasAuthority()) return;
	if (bIsDead) return;
	CurrentHP -= Amount;
	UE_LOG(LogTemp, Warning, TEXT("TakeDamage!"));
	UE_LOG(LogTemp,Warning,TEXT("Enemy Attacked : %f, Max HP : %f"),CurrentHP, MaxHP);
	OnRep_CurrentHP();

	if (CurrentHP <= 0.f)
	{
		Die();
	}
}

void UCreatureCombatComponent::OnRep_CurrentHP()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] CCC::OnRep_CurrentHP fired. Broadcasting OnHealthChanged."), 
		  GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
	// ✨ 체력이 변경되었음을 외부에 알리는 신호를 보냅니다(Broadcast).
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void UCreatureCombatComponent::Die()
{
	// 반드시 서버에서만 로직 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_Die();
		return;
	}
	
	if (bIsDead) return;
	bIsDead = true;
	//OnDied.Broadcast();

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		OwnerChar->DetachFromControllerPendingDestroy();
		OwnerChar->SetLifeSpan(5.f);
		OwnerChar->GetMesh()->SetSimulatePhysics(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Die(): OwnerChar 캐스팅 실패!"));
		return;
	}

	// 아이템 소환 로직, 블루프린트에서 뭘 소환할지 지정해주어야 함.
	if (DropItemClass)
	{
		FVector DropLocation = OwnerChar->GetActorLocation() + FVector(0.f, 0.f, -30.f);
		FRotator DropRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* DroppedItem = GetWorld()->SpawnActor<AActor>(DropItemClass, DropLocation, DropRotation, SpawnParams);
	
		OwnerChar->Destroy(); // 나중에 사망 애니메이션이나 이펙트 등을 보여주고 싶으면 Multicast RPC로 복제해야함.
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Die(): DropItemClass is null! 아이템 드롭 불가"));
		return;
	}
}

void UCreatureCombatComponent::Server_Die_Implementation()
{
	Die();
}

float UCreatureCombatComponent::GetHealthPercent() const
{
	return (MaxHP > 0.f) ? CurrentHP / MaxHP : 0.f;
}

void UCreatureCombatComponent::InitStats(float InMaxHP)
{
	MaxHP    = FMath::Max(1.f, InMaxHP);
	CurrentHP = MaxHP;
	bIsDead   = false;
	OnRep_CurrentHP();
}
