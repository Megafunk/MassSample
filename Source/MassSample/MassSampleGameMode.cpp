// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassSampleGameMode.h"
#include "MassSampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMassSampleGameMode::AMassSampleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
