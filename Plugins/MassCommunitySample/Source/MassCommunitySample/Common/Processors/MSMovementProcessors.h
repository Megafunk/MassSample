// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MassProcessor.h"
#include "MSMovementProcessors.generated.h"

// FIXME: This hello world is perfect as is, we just need to investigate the velocity fragment missbehav and document it.

/**
 * Example processor demonstrating how to move entities found in query
 * Processors are called "Systems" in most ECS libraries. 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSGravityProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:

	UMSGravityProcessor();

protected:

	//Note that we declare this ourselves! You can have many queries if need be.
	FMassEntityQuery GravityEntityQuery;

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};

/**
 * Example processor demonstrating how to move entities found in query
 * Processors are called "Systems" in most ECS libraries. 
 */

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSBasicMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:

	UMSBasicMovementProcessor();


	FMassEntityQuery MovementEntityQuery;
	
	FMassEntityQuery RotationFollowsVelocity;
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};

