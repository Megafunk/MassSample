// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "RTSAgentProcessors.generated.h"

// Defines that an entity is managed by the RTSHashGrid
USTRUCT()
struct FRTSAgentHashTag : public FMassTag
{
	GENERATED_BODY();
};

// Update hash grid position of entities
UCLASS()
class RTSFORMATIONS_API URTSUpdateHashPosition : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;
};

// Initialize the hash grid position of RTS agents
UCLASS()
class RTSFORMATIONS_API URTSInitializeHashPosition : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSInitializeHashPosition();
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;
};

// Remove the entities from the hash grid when destroyed/no longer an RTSAgent
UCLASS()
class RTSFORMATIONS_API URTSRemoveHashPosition : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSRemoveHashPosition();
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;
};
