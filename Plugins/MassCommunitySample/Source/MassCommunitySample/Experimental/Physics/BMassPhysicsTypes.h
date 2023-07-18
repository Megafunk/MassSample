// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "BMassPhysicsTypes.generated.h"

// Handle to our physics thread particle?
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSMassPhysicsFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FPhysicsActorHandle SingleParticlePhysicsProxy;
};
