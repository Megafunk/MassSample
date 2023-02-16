// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MSFragments.generated.h"

/**
 * FMassFragments are simple structs that can hold data.
 * If you want to, it is possible to add functions as well for getter/setters and the like.
 * USTRUCTS cannot have conventional UFunctions though
 */

// A simple color fragment used in many examples
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FSampleColorFragment : public FMassFragment
{
	GENERATED_BODY()

	FSampleColorFragment() = default;
	
	FSampleColorFragment(FColor Color) : Color(Color){};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FColor Color = FColor::Red;
};


USTRUCT(BlueprintType)
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


USTRUCT(BlueprintType)
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
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSGravityTag : public FMassTag
{
	GENERATED_BODY()
};
// This one is used in UMSBasicMovementProcessor for the RotationFollowsVelocity query
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSRotationFollowsVelocityTag : public FMassTag
{
	GENERATED_BODY()
};
// Indicates we want to use the Mass Sample provided movement processor  (uses force to change velocity and transform)
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSBasicMovement : public FMassTag
{
	GENERATED_BODY()
};


USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMassSampleDebuggableTag : public FMassTag
{
	GENERATED_BODY()
};


// A simple shared fragment example
// Generally these are added through traits

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSExampleSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	double SomeKindaOfData = 0.f;
};








USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSAngularForce : public FMassFragment
{
	GENERATED_BODY()
	FVector Value = FVector::ZeroVector;
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSAngularVelocity : public FMassFragment
{
	GENERATED_BODY()
	FVector Value = FVector::ZeroVector;
};

/**
 * Signal names
 */
namespace MassSample::Signals
{
	static const FName OnProjectileHitSomething = FName("SampleOnCollide");
	static const FName OnGetHit = FName("SampleOnHit");
}


