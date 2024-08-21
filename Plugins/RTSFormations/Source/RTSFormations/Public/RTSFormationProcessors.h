#pragma once
#include "MassEntityQuery.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "RTSFormationProcessors.generated.h"

class URTSFormationSubsystem;
const FName FormationUpdated = FName(TEXT("FormationUpdated"));
const FName UpdateIndex = FName(TEXT("UpdateIndex"));

// Observer that runs when a unit is spawned. Its main purpose is to add entities to a unit array
// in the subsystem and cache the index for future use in URTSFormationUpdate
UCLASS()
class RTSFORMATIONS_API URTSFormationInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationInitializer();
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};

// Observer that runs when an entity is destroyed. Cleans up the unit array and tells the last unit to take their place
UCLASS()
class RTSFORMATIONS_API URTSFormationDestroyer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationDestroyer();
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};


// Simple movement processor to get agents from a to b
UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	FMassEntityQuery FormationQuery;
};

// Main bulk of formation logic. Calculates position of entities and sends it to the FMassMoveTargetFragment.
UCLASS()
class RTSFORMATIONS_API URTSFormationUpdate : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};

// Signal Processor that updates the agents index in the unit based on values in the FormationSubsystem Unit Array
UCLASS()
class RTSFORMATIONS_API URTSUpdateEntityIndex : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};
