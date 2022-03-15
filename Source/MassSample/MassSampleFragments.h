// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassSampleFragments.generated.h"

/**
 * FMassFragments are simple structs that can hold data.
 * If you want to, it is possible to add functions as well for getter/setters and the like.
 */
USTRUCT()
struct FSampleFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FColor Color = FColor::Red;
};

//this already exists in the mass movement module but I just wanted to demonstrate
USTRUCT()
struct FSampleVelocityFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector Value = FVector::ZeroVector;
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