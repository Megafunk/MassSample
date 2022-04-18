// Fill out your copyright notice in the Description page of Project Settings.


#include "ZoneGraphExample/Tasks/MSShowLanePositionTask.h"

#include "StateTreeExecutionContext.h"
#include "ZoneGraphSubsystem.h"
#include "MassEntitySubsystem.h"

#include "MassEntityTypes.h"

#include "MassActorSubsystem.h"

#include "MassCommonTypes.h"

#include "ZoneGraphExample/MSEntityActorExampleSubsystem.h"

#include "MassZoneGraphAnnotationTypes.h"
#include "MassZoneGraphNavigationTypes.h"


bool FShowLanePositionTask::Link(FStateTreeLinker& Linker)
{

	
	Linker.LinkExternalData(Path2PathSubsystemHandle);
	Linker.LinkExternalData(MassEntitySubsystemHandle);
	Linker.LinkExternalData(ZoneGraphSubsystemHandle);
	

	// NOTE: Handle – Reference to TStateTreeExternalDataHandle<> with USTRUCT type to link to.
	Linker.LinkExternalData(ZoneGraphLaneLocationFragmentHandle);



	Linker.LinkInstanceDataProperty(TargetLocationHandle, STATETREE_INSTANCEDATA_PROPERTY(FShowLanePositionTaskData, TargetLocation));

	return true;
}

EStateTreeRunStatus FShowLanePositionTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	
	UMassEntitySubsystem& MassEntitySubsystem = Context.GetExternalData(MassEntitySubsystemHandle);
	const UZoneGraphSubsystem& ZoneGraphSubsystem = Context.GetExternalData(ZoneGraphSubsystemHandle);
	
	

	FMassZoneGraphTargetLocation& TargetLocation = Context.GetInstanceData(TargetLocationHandle);

	FZoneGraphLaneHandle& LaneHandle = TargetLocation.LaneHandle;

	if (!LaneHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LaneHandle is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	const FZoneGraphStorage* GraphStorage = ZoneGraphSubsystem.GetZoneGraphStorage(LaneHandle.DataHandle);



	if (!GraphStorage->Lanes.IsValidIndex(LaneHandle.Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("!GraphStorage->Lanes.IsValidIndex(LaneHandle.Index)"));
		return EStateTreeRunStatus::Failed;
	}

	
	// Highlight the segment the entity is on
	const FZoneLaneData DestZoneLaneData = GraphStorage->Lanes[LaneHandle.Index];
	for (int32 i = DestZoneLaneData.PointsBegin; i < DestZoneLaneData.PointsEnd - 1; i++)
	{
		const FVector& SegStart = GraphStorage->LanePoints[i];
		const FVector& SegEnd = GraphStorage->LanePoints[i + 1];
		const UWorld* World = &ZoneGraphSubsystem.GetWorldRef();
		DrawDebugLine(World, SegStart, SegEnd, FColor::Orange, false, 5.0f, 2, 8.f);
	}



	TargetLocation.LaneHandle = LaneHandle;




	return EStateTreeRunStatus::Succeeded;
	
}