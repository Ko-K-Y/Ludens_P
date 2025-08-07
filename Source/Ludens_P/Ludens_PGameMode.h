// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Ludens_PGameMode.generated.h"



UCLASS(minimalapi)
class ALudens_PGameMode : public AGameModeBase
{
	GENERATED_BODY()

	//플레이어 시작 위치 받기
	UPROPERTY()
	TArray<AActor*> UsedStartSpots;

	//적 생성
public:

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override; //ChoosePlayerStart


	ALudens_PGameMode();
	void BeginPlay();
};

