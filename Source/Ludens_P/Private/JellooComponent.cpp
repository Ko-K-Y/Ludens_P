// Fill out your copyright notice in the Description page of Project Settings.


#include "JellooComponent.h"

// Sets default values for this component's properties
UJellooComponent::UJellooComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
	SetIsReplicatedByDefault(true);
	/*MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	
	//MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetIsReplicated(true);*/
}


// Called when the game starts
void UJellooComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentJellooHP = MaxJellooHP;
}


// Called every frame
void UJellooComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UJellooComponent::OnRep_JellooHP()
{
	// CurrentJellooHP 값이 바뀌었을 때 처리 UI 처리
}

void UJellooComponent::JellooTakeDamage(float Amount)
{
	if (!GetOwner()->HasAuthority()) return;
	if (bIsDead) return;
	CurrentJellooHP -= Amount;
	UE_LOG(LogTemp,Warning,TEXT("Jelloo Absorbed : %hd"),CurrentJellooHP);
	OnRep_JellooHP();

	if (CurrentJellooHP <= 0.f)
	{
		DestroyJelloo();
	}
}

void UJellooComponent::DestroyJelloo()
{
	// 반드시 서버에서만 로직 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_DestroyJelloo();
		return;
	}
	bIsDead = true;
	AActor* OwnerChar = Cast<AActor>(GetOwner());
	if (!OwnerChar)
	{
		UE_LOG(LogTemp, Error, TEXT("DestroyJelloo(): 소유자 액터가 없습니다."));
	}
	
	OwnerChar->Destroy();
}

void UJellooComponent::Server_DestroyJelloo_Implementation()
{
	DestroyJelloo();
}

void UJellooComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UJellooComponent, MaxJellooHP);
	DOREPLIFETIME(UJellooComponent, CurrentJellooHP);
}
