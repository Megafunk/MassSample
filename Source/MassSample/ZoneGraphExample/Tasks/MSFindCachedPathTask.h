// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MassStateTreeTypes.h"
#include "ZoneGraphTypes.h"

#include "Tasks/MassZoneGraphPathFollowTask.h"
#include "ZoneGraphExample/Fragments/MSZoneGraphExampleFragments.h"

#include "MSFindCachedPathTask.generated.h"






class UZoneGraphAnnotationSubsystem;
class UMassCrowdSubsystem;
class UZoneGraphSubsystem;


/*
 * This is a MassAI Behaviour struct, properties here catagory will be exposed to the StateTree editor.
 * These properties can be used for output or input.
 * You can bind input properties to the outputs provided by other nodes in the tree (often other tasks and evaluaters)
 */
USTRUCT()
struct MASSSAMPLE_API FFindCachedPathTaskData
{
	GENERATED_BODY()

	/** This is the output FMassZoneGraphTargetLocation, it is similar to a normal ZoneGraphLaneLocation struct, however it used by the "Follow path" StateTree task to move*/
	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassZoneGraphTargetLocation FollowPathTargetLocation;
	
};



/**
 * This is task node will use the "PathFromFragmentHandle" to get the values that define the path to target (paths and values are calculated in "UMSPathFindingProcessor")
 * It will then use the values in the TMap<int(LaneIndex), int(LaneLinkIndex)> to find the next ZoneGraph LaneLink to move towards.
 * The output is a "FMassZoneGraphTargetLocation" that can be used in the ZoneGraph FollowPath node to follow path.
 */
USTRUCT(meta = (DisplayName = "MS Find Cached Path"))
struct MASSSAMPLE_API FFindCachedPathTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	FFindCachedPathTask();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FFindCachedPathTaskData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;

	
	//TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> LocationHandle;
	TStateTreeExternalDataHandle<UZoneGraphSubsystem> ZoneGraphSubsystemHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> MassEntitySubsystemHandle;
	TStateTreeExternalDataHandle<UZoneGraphAnnotationSubsystem> ZoneGraphAnnotationSubsystemHandle;
	TStateTreeExternalDataHandle<UMassCrowdSubsystem> MassCrowdSubsystemHandle;


	TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> LocationHandle;

	
	TStateTreeInstanceDataPropertyHandle<FMassZoneGraphTargetLocation> FollowPathTargetLocationHandle;

	TStateTreeExternalDataHandle<FZoneGraphPathTestFromFragment> PathFromFragmentHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FZoneGraphTag DisturbanceAnnotationTag = FZoneGraphTag::None;

	
};
