// Copyright Epic Games, Inc. All Rights Reserved.

#include "MSGameMode.h"
#include "Character/MSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMSGameMode::AMSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
