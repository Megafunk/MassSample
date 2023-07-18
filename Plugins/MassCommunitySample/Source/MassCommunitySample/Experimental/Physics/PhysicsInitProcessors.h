// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "PhysicsInitProcessors.generated.h"

/**
 * Inits new physics objects to the chaos simulation when a FMSMassPhysicsFragment is added
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSPhysicsInitProcessors : public UMassObserverProcessor
{
	GENERATED_BODY()
	UMSPhysicsInitProcessors();
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext) override;

	FMassEntityQuery EntityQuery;

};
