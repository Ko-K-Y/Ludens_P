// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttackHandler.h"
#include "Camera/CameraComponent.h"
#include "CreatureCombatComponent.h"
#include "JellooComponent.h"
#include "GameFramework/Character.h"
#include "Ludens_P/Ludens_PCharacter.h"

void UMeleeAttackHandler::BeginPlay()
{

}


void UMeleeAttackHandler::HandleMeleeAttack(float damage)
{
	UE_LOG(LogTemp, Display, TEXT("HandleMeleeAttack!"));

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is nullptr!"));
		return;
	}
	// 근접 공격 로직 -> 공격 로직, 이펙트, 사운드 등등
	
	if (damage) // 나중에 적이 흰색인 경우로 제한
	{
		ALudens_PCharacter* LudensCharacter = Cast<ALudens_PCharacter>(OwnerCharacter);
		if (!LudensCharacter)
		{
			UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is not ALudens_PCharacter!"));
			return;
		}
		
		// 화면 중심에서 월드 방향 구하기
		FVector WorldLocation = LudensCharacter->FirstPersonCameraComponent->GetComponentLocation();
		FRotator CameraRotation = OwnerCharacter->GetActorRotation();
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
		}

		// 트레이스 시작/끝 위치 계산
		FVector TraceDirection = CameraRotation.Vector();
		FVector TraceStart = WorldLocation;
		FVector TraceEnd = TraceStart + (TraceDirection * MeleeRange);

		// 라인 트레이스
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerCharacter);

		// 라인 트레이스를 하여 무언가에 맞았는지를 나타냄
		bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params);

		// 6. CreatureCombatComponent가 있으면 데미지 적용
		if (bHit && Hit.GetActor())
		{
			// 맞은 액터가 CCC를 가지고 있는지 검사
			if (UCreatureCombatComponent* CombatComp = Hit.GetActor()->FindComponentByClass<UCreatureCombatComponent>())
			{
				CombatComp->TakeDamage(damage);
			}
		}
	}
}
