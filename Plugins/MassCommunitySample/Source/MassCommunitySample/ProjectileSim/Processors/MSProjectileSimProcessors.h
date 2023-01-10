// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassMovementFragments.h"
#include "MassProcessor.h"
#include "MSProjectileSimProcessors.generated.h"
/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSProjectileSimLineTrace : public UMassProcessor
{
	GENERATED_BODY()

	UMSProjectileSimLineTrace();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery ProjectileOctreeQuery;
	FMassEntityQuery LineTraceFromPreviousPosition;
	FMassEntityQuery RotationFollowsVelocity;

};


UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSProjectileSimOctreeQueryProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UMSProjectileSimOctreeQueryProcessor();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery ProjectileOctreeQuery;
};