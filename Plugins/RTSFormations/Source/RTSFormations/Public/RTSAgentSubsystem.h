// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HierarchicalHashGrid2D.h"
#include "MassEntityTypes.h"
#include "RTSAgentSubsystem.generated.h"


typedef THierarchicalHashGrid2D<2, 4, FMassEntityHandle> RTSAgentHashGrid2D;

/**
 * 
 */
UCLASS()
class RTSFORMATIONS_API URTSAgentSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	RTSAgentHashGrid2D AgentHashGrid = RTSAgentHashGrid2D();

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void LaunchEntities(const FVector& Location, float Radius) const;
};
