#include "EnemyBase.h"

#include "EnemyAIController.h"
#include "ShooterAIComponent.h"
#include "StealthComponent.h"
#include "TP_WeaponComponent.h"
#include "Net/UnrealNetwork.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	NetDormancy = DORM_Never;

	
	Combat = CreateDefaultSubobject<UCreatureCombatComponent>(TEXT("CombatComponent"));
	
	bAlwaysRelevant = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyBase, EnemyColor);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 100.0f; 
	MinNetUpdateFrequency = 50.0f;

	if (!Combat)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Combat is not assigned in %s!"), *GetName());
	}
	
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AEnemyBase::SetActive(bool bNewActive)
{
	bActive = bNewActive;

	SetActorHiddenInGame(!bNewActive);
	SetActorEnableCollision(bNewActive);
	SetActorTickEnabled(bNewActive);
	StealthComponent = FindComponentByClass<UStealthComponent>();

	if (Combat) 
	{
		Combat->SetComponentTickEnabled(bNewActive);
	}
	else
	{
		ensureAlwaysMsgf(false, TEXT("EnemyBase must have CombatComponent initialized!"));
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetVisibility(bNewActive, true);
		MeshComp->SetHiddenInGame(!bNewActive, true);
		MeshComp->bPauseAnims = !bNewActive;
		MeshComp->SetComponentTickEnabled(bNewActive);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ GetMesh() returned null!"));
	}
	TArray<UActorComponent*> AIComponents;
	GetComponents(UActorComponent::StaticClass(), AIComponents);
	for (UActorComponent* Component : AIComponents)
	{
		if (Component->IsA<UWalkerAIComponent>() || Component->IsA<UShooterAIComponent>())
		{
			Component->SetComponentTickEnabled(bNewActive);
		}
	}

	if (StealthComponent && bActive)
	{
		StealthComponent->ResetStealthState();
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("No StealthComponent"));
	}
}

void AEnemyBase::PostNetInit()
{
	Super::PostNetInit();

	if (!HasAuthority())
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyBase::OnPostReplicationInit, 0.1f, false);
	}
}

void AEnemyBase::OnPostReplicationInit()
{
	SetActive(false); 
}
bool AEnemyBase::IsActive() const
{
	return bActive;
}
void AEnemyBase::MulticastSetActive_Implementation(bool bNewActive)
{
	SetActive(bNewActive);
}

void AEnemyBase::SetupEnemyForColor(EEnemyColor NewColor)
{
	if (HasAuthority())
	{
		EnemyColor = NewColor;
		
		InitializeShields();
		
		// 서버에서도 즉시 머티리얼이 보이도록 직접 호출
		UpdateMaterial();
	}
}

void AEnemyBase::OnRep_EnemyColor()
{
	// EnemyColor 변수가 클라이언트에 복제되었을 때 호출됩니다.
	UpdateMaterial();
}

void AEnemyBase::UpdateMaterial()
{
	if (EnemyMaterials.Contains(EnemyColor))
	{
		UMaterialInstance* FoundMaterial = EnemyMaterials[EnemyColor];
		if (FoundMaterial && GetMesh())
		{
			GetMesh()->SetMaterial(MaterialSlotIndex, FoundMaterial);
		}
	}
}

void AEnemyBase::InitializeShields()
{
	// 기존 쉴드 정보를 모두 지웁니다.
	Shields.Empty();

	// DefaultShields 맵에 정의된 각 기본 색상에 대해 반복합니다.
	for (const TPair<EEnemyColor, int32>& ShieldInfo : DefaultShields)
	{
		const EEnemyColor& Color = ShieldInfo.Key;
		const int32& MaxShieldAmount = ShieldInfo.Value;

		// 이 적의 EnemyColor가 해당 기본 색상을 포함하는지 비트 연산으로 확인합니다.
		if (EnumHasAllFlags(EnemyColor, Color))
		{
			// 포함한다면, 현재 쉴드 맵(Shields)에 최대치로 추가합니다.
			Shields.Add(Color, MaxShieldAmount);
		}
	}
}

void AEnemyBase::ApplyShieldDamage(EEnemyColor DamageColor, int32 DamageAmount)
{
	// 데미지 타입에 해당하는 쉴드가 있는지 확인합니다.
	if (int32* CurrentShield = Shields.Find(DamageColor))
	{
		// 쉴드가 있다면 데미지를 적용합니다.
		*CurrentShield -= DamageAmount;

		// 쉴드가 0 이하로 떨어졌다면 맵에서 제거합니다.
		if (*CurrentShield <= 0)
		{
			Shields.Remove(DamageColor);
			// 여기에 쉴드가 파괴되었을 때의 로직을 추가할 수 있습니다 (예: 이펙트 재생)
		}
	}
	else
	{
		// 해당 색상의 쉴드가 없으면, 본체(Health)에 데미지를 줍니다.
		// ... (본체 데미지 로직)
	}
}
