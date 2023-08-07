// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MSPhysicsInitProcessors.generated.h"

/**
 * Inits new physics objects to the chaos simulation when a FMSMassPhysicsFragment is added
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSPhysicsInitProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UMSPhysicsInitProcessor();
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext) override;

	FMassEntityQuery EntityQuery;
};
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSPhysicsCleanupProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UMSPhysicsCleanupProcessor();
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext) override;

	FMassEntityQuery EntityQuery;
};

