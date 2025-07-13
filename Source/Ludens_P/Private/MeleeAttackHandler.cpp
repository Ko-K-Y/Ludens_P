// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttackHandler.h"

#include "CreatureCombatComponent.h"
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
		// 1. 플레이어 컨트롤러 얻기
		APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (!PC) return;

		// 2. 화면 중앙 좌표 구하기
		int32 ViewportX, ViewportY;
		PC->GetViewportSize(ViewportX, ViewportY);
		FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

		// 3. 화면 중심에서 월드 방향 구하기
		FVector WorldLocation, WorldDirection;
		if (!PC->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
			return;

		// 4. 트레이스 시작/끝 위치 계산
		FVector TraceStart = WorldLocation;
		FVector TraceEnd = TraceStart + (WorldDirection * MeleeRange);

		// 5. 라인 트레이스
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
