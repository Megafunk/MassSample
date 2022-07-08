// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "MassObserverProcessor.h"
#include "RTSAgentTraits.generated.h"

// Store basic info about the unit
USTRUCT()
struct RTSFORMATIONS_API FRTSFormationAgent : public FMassFragment
{
	GENERATED_BODY()

	// The index of the unit in the formation
	int UnitIndex = 0;
};

UCLASS()
class RTSFORMATIONS_API URTSFormationAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};

// Ensure that units are in formation
UCLASS()
class RTSFORMATIONS_API URTSFormationInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationInitializer();
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	FMassEntityQuery MoveEntityQuery;
};

UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
