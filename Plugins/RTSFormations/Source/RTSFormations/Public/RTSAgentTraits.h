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

USTRUCT()
struct RTSFORMATIONS_API FRTSFormationSettings : public FMassSharedFragment
{
	GENERATED_BODY()

	// Distance between each unit
	UPROPERTY(EditAnywhere, Category = "Formation")
	float BufferDistance = 100.f;

	// Width ratio for formation
	UPROPERTY(EditAnywhere, Category = "Formation")
	int UnitRatioX = 1;

	// Length ratio for formation
	UPROPERTY(EditAnywhere, Category = "Formation")
	int UnitRatioY = 1;
};

// Provides entity with FRTSFormationAgent fragment to enable formations
UCLASS()
class RTSFORMATIONS_API URTSFormationAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};

// Observer that runs when a unit is spawned. Calculates square formation position based on unit count
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

// Simple movement processor to get agents from a to b
UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
