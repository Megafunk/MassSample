// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MassEntityTypes.h"
#include "MSNavMeshFragments.generated.h"


USTRUCT()
struct MASSSAMPLE_API FNavMeshAIFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FVector NextPathNodePos;
	int NextPathNodeIndex;
};
