
#pragma once

#include "MassEntityQuery.h"
#include "MassEntityTypes.h"
#include "MassSignalProcessorBase.h"
#include "MassSignalSubsystem.h"
#include "RTSFormationSubsystem.h"
#include "LaunchEntityProcessor.generated.h"

const FName LaunchEntity = FName(TEXT("LaunchEntity"));

// Observer that runs when an entity is destroyed. Cleans up the unit array and tells the last unit to take their place
UCLASS()
class RTSFORMATIONS_API ULaunchEntityProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void SignalEntities(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

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
	
	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float Magnitude = 500.f;
};

USTRUCT()
struct RTSFORMATIONS_API FInitLaunchFragment : public FMassTag
{
	GENERATED_BODY()
};
