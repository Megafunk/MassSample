// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "MSMassPhysicsTypes.generated.h"

// Handle to our physics thread particle?
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSMassPhysicsFragment : public FMassFragment
{
	GENERATED_BODY()
	// Note: the term "Actor" here means chaos physics actor handle, which is not a uobject unreal actor...
	FPhysicsActorHandle SingleParticlePhysicsProxy;
};


USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSChaosToMassTag : public FMassTag
{GENERATED_BODY()};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSMassToChaosTag : public FMassTag
{GENERATED_BODY()};