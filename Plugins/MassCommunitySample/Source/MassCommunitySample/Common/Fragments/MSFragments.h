// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Evaluation/MovieScenePlayback.h"
#include "MSFragments.generated.h"

/**
 * FMassFragments are simple structs that can hold data.
 * If you want to, it is possible to add functions as well for getter/setters and the like.
 */
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FSampleColorFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	FColor Color = FColor::Red;
};

USTRUCT()
struct FISMPerInstanceDataFragment : public FMassFragment
{
	GENERATED_BODY()
	float data = 0;
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FInterpLocationFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector StartingLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	float Duration = 1.0f;
	
	bool bForwardDirection = true;

	float Time = 0.0f;
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FOriginalTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FTransform Transform;
};

/**
 * FMassTags are structs that exist only as tags on Mass entities. They cannot have data!
 * Mostly used for filtering. They can be queried for to only include certain entities etc.
 */
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FSampleMoverTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMassSampleDebuggableTag : public FMassTag
{
	GENERATED_BODY()
};


/**
 * Signal names
 */
namespace MassSample::Signals
{
	const FName OnHit = FName(TEXT("SampleOnHit"));
}


