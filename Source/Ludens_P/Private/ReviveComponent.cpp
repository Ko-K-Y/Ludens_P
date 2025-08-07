// Fill out your copyright notice in the Description page of Project Settings.


#include "ReviveComponent.h"

#include "Ludens_P/Ludens_PCharacter.h"
#include "GameFramework/Character.h"
#include "PlayerStateComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// 플레이어 부활 관련 컴포넌트

// Sets default values for this component's properties
UReviveComponent::UReviveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
	// ...
}


// Called when the game starts
void UReviveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	PlayerStateComp = Cast<UPlayerStateComponent>(GetOwner());
}

void UReviveComponent::HandleRevive()
{
	// 팀원 소생 로직, UI, 사운드 등 담당
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter X"))
		return;
	}
	ALudens_PCharacter* Ludens_PCharacter = Cast<ALudens_PCharacter>(GetOwner());
	
	// 화면 중심에서 월드 방향 구하기
	FVector WorldLocation = Ludens_PCharacter->FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRotation = OwnerCharacter->GetActorRotation();
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	}
	else UE_LOG(LogTemp, Warning, TEXT("❗ GetController() is null, fallback to actor rotation"));

	FVector TraceDirection = CameraRotation.Vector();
	// 트레이스 시작/끝 위치 계산
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = TraceStart + (TraceDirection * 100.f);

	// 라인 트레이스
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	// 라인 트레이스를 하여 무언가에 맞았는지를 나타냄
	bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params);

	if (bHit && Hit.GetActor())
	{
		if (UPlayerStateComponent* PlayerState = Hit.GetActor()->FindComponentByClass<UPlayerStateComponent>())
		{
			TargetPlayerState = PlayerState;
			if (TargetPlayerState->IsKnocked)
			{
				GetWorld()->GetTimerManager().SetTimer(ReviveTimer, this, &UReviveComponent::HandleReviveComplete, 5.f, false);
				TargetPlayerState->GetWorld()->GetTimerManager().PauseTimer(TargetPlayerState->KnockedTimer);
			}
		}
	} 
}

void UReviveComponent::HandleReviveComplete()
{
	// 소생 완료 후 타이머 클리어, 변수 초기화 등 추가 가능
	TargetPlayerState->CurrentHP = (TargetPlayerState->MaxHP) / 2;
	TargetPlayerState->IsKnocked = false;
	TargetPlayerState->UpdateMoveSpeed();
	UE_LOG(LogTemp, Error, TEXT("Revive Complete!"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentHP: %f"), TargetPlayerState->CurrentHP);
	TargetPlayerState = nullptr;
}


// Called every frame
void UReviveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UReviveComponent::IsReviving() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(ReviveTimer);
}

void UReviveComponent::CancelRevive()
{
	// 소생 취소 조건(입력 등)을 별도로 체크해서 타이머 Clear/UnPause 처리
	UE_LOG(LogTemp, Warning, TEXT("CancelRevive"))
	if (TargetPlayerState)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReviveTimer);
		TargetPlayerState->GetWorld()->GetTimerManager().UnPauseTimer(TargetPlayerState->KnockedTimer);
		TargetPlayerState = nullptr;
	}
}

