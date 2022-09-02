
#pragma once

#include "MassEntityQuery.h"
#include "MassEntityTypes.h"
#include "MassObserverProcessor.h"
#include "MassSignalSubsystem.h"
#include "RTSFormationSubsystem.h"
#include "LaunchEntityProcessor.generated.h"

// Observer that runs when an entity is destroyed. Cleans up the unit array and tells the last unit to take their place
UCLASS()
class RTSFORMATIONS_API ULaunchEntityProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	ULaunchEntityProcessor();
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};

// Observer that runs when an entity is destroyed. Cleans up the unit array and tells the last unit to take their place
UCLASS()
class RTSFORMATIONS_API UMoveForceProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct RTSFORMATIONS_API FLaunchEntityFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float Magnitude;
};