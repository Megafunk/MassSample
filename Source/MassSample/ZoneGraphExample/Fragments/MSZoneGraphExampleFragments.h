// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MassEntityTypes.h"
#include "ZoneGraphTypes.h"

#include "MSZoneGraphExampleFragments.generated.h"



// Test AI Stuff


USTRUCT()
struct MASSSAMPLE_API FZoneGraphPathTestFromFragment : public FMassFragment
{
	GENERATED_BODY()

	FTransform FromTransform;

	TArray<FZoneGraphLinkedLane> OutPathLinkedLanes;

	bool CurrentlyCachedMovement = false;
	
};

USTRUCT()
struct MASSSAMPLE_API FZoneGraphPathTestToFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FTransform ToTransform;
};