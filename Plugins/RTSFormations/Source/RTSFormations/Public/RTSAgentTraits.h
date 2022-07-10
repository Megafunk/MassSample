// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "RTSAgentTraits.generated.h"

class URTSFormationSubsystem;

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

	// Unit width of formation
	UPROPERTY(EditAnywhere, Category = "Formation")
	int FormationLength = 1;
};

// Provides entity with FRTSFormationAgent fragment to enable formations
UCLASS()
class RTSFORMATIONS_API URTSFormationAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	FRTSFormationSettings FormationSettings;
};

//@todo create destroyer observer processor to handle updating units
//@todo create another processor to handle when units need to be updated (using bUpdateUnitPosition in SharedFragment)
//@todo I definitely went overboard in terms of complexity, this example should really just be straightforward to read and have performance second i think
