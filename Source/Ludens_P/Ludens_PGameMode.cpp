// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ludens_PGameMode.h"
#include "Ludens_PCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALudens_PGameMode::ALudens_PGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
