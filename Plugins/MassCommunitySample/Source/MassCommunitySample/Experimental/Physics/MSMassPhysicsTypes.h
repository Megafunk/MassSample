// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "MSMassPhysicsTypes.generated.h"

// Handle to a physics thread particle
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSMassPhysicsFragment : public FMassFragment
{
	GENERATED_BODY()

	FMSMassPhysicsFragment() = default;
	// Note: the term "Actor" here is not a typical unreal AActor but a pointer to a chaos physics particle
	FPhysicsActorHandle SingleParticlePhysicsProxy = nullptr;


	FMSMassPhysicsFragment(const FPhysicsActorHandle& ParticlePhysicsProxy)
	{
		SingleParticlePhysicsProxy = ParticlePhysicsProxy;
	};
};

// Imitates the collision shapes a static mesh would normally have
USTRUCT(BlueprintType)
struct FSharedCollisionSettingsFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FBodyInstance BodyInstance;
	// these arrays are all editfixed size... blegh
	UPROPERTY(EditAnywhere)
	FKAggregateGeom Geometry;
};

// Whether we are kinematic or simulating physics
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSSimulatesPhysicsTag : public FMassTag
{
	GENERATED_BODY()
};

/* Inits new choas bodies with bUpdateKinematicFromSimulation = true
 * which is VERY NEW and is on main only as of writing?
 * I think what this does is do an actual solve to move the bodies around with the SetKinematicTarget stuff
 * 
 */
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSUpdateKinematicFromSimulationTag : public FMassTag
{
	GENERATED_BODY()
};

// Chaos moves our mass transform
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSChaosToMassTag : public FMassTag
{
	GENERATED_BODY()
};

// Mass moves our chaos transform (on the game thread...)
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSMassToChaosTag : public FMassTag
{
	GENERATED_BODY()
};
