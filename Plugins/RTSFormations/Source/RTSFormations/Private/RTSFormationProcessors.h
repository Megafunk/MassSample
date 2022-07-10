#pragma once
#include "MassEntityQuery.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "RTSFormationProcessors.generated.h"

class URTSFormationSubsystem;
const FName FormationUpdated = FName(TEXT("FormationUpdated"));

// Observer that runs when a unit is spawned. Calculates square formation position based on unit count
UCLASS()
class RTSFORMATIONS_API URTSFormationInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationInitializer();
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void Register() override;

	FMassEntityQuery EntityQuery;

	FMassEntityQuery MoveEntityQuery;

	FMassEntityQuery DestroyQuery;

	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
	
	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};

// Simple movement processor to get agents from a to b
UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	FMassEntityQuery FormationQuery;
};

UCLASS()
class RTSFORMATIONS_API URTSFormationUpdate : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};
