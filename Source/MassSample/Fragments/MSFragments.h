// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MSFragments.generated.h"

/**
 * FMassFragments are simple structs that can hold data.
 * If you want to, it is possible to add functions as well for getter/setters and the like.
 */
USTRUCT()
struct FSampleColorFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FColor Color = FColor::Red;
};

/**
 * FMassTags are structs that exist only as tags on Mass entities. They cannot have data!
 * Mostly used for filtering. They can be queried for to only include certain entities etc.
 */
//*** TAGS ***//
USTRUCT()
struct FMoverTag : public FMassTag
{
	GENERATED_BODY()
};
USTRUCT()
struct FMassSampleDebuggableTag : public FMassTag
{
	GENERATED_BODY()
};

