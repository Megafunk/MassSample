// Fill out your copyright notice in the Description page of Project Settings.


#include "ZoneGraphExample/Tasks/MSFindCachedPathTask.h"


#include "MassStateTreeExecutionContext.h"
#include "ZoneGraphAnnotationSubsystem.h"

#include "MassCrowdSubsystem.h"
#include "MassZoneGraphNavigationFragments.h"
#include "ZoneGraphSubsystem.h"

#include "ZoneGraphQuery.h"

#include "MassAIBehaviorTypes.h"
#include "MassCrowdSettings.h"


FFindCachedPathTask::FFindCachedPathTask()
{
}

bool FFindCachedPathTask::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkExternalData(LocationHandle);
	Linker.LinkExternalData(ZoneGraphSubsystemHandle);
	Linker.LinkExternalData(ZoneGraphAnnotationSubsystemHandle);
	Linker.LinkExternalData(MassCrowdSubsystemHandle);
	Linker.LinkExternalData(MassEntitySubsystemHandle);

	Linker.LinkExternalData(LocationHandle);

	Linker.LinkExternalData(PathFromFragmentHandle);


	Linker.LinkInstanceDataProperty(FollowPathTargetLocationHandle, STATETREE_INSTANCEDATA_PROPERTY(FFindCachedPathTaskData, FollowPathTargetLocation));
	
	
	return true;
}


// TODO: Make the running of this state return a EStateTreeRunStatus::Failed if a certain event/condition is met (prob through mass),
// TODO: this would allow for something such as avoidance to properly exit out of this state. Combining this with conditions could make some interesting stuff.

EStateTreeRunStatus FFindCachedPathTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	bool bDisplayDebug = false;

	UE_LOG( LogTemp, Warning, TEXT("FFindZoneGraphPathPosition - RUNNING"));

#if WITH_MASSGAMEPLAY_DEBUG
	bDisplayDebug = UE::Mass::Debug::IsDebuggingEntity(MassContext.GetEntity());
#endif // WITH_MASSGAMEPLAY_DEBUG


	// Get in referenced systems
	UZoneGraphSubsystem& ZoneGraphSubsystem = Context.GetExternalData(ZoneGraphSubsystemHandle);
	UZoneGraphAnnotationSubsystem& ZoneGraphAnnotationSubsystem = Context.GetExternalData(ZoneGraphAnnotationSubsystemHandle);
	UMassCrowdSubsystem& MassCrowdSubsystem = Context.GetExternalData(MassCrowdSubsystemHandle);
	UMassEntitySubsystem& MassEntitySubsystem = Context.GetExternalData(MassEntitySubsystemHandle);
	

	// This entities lane location
	const FMassZoneGraphLaneLocationFragment& LaneLocation = Context.GetExternalData(LocationHandle);


	// Get from fragment to retrieve cached pathing data
	FZoneGraphPathTestFromFragment& ZoneGraphPathTestFromFragment = Context.GetExternalData(PathFromFragmentHandle);

	TMap<int, FZoneGraphLinkedLane>& CachedLinkedLanes = ZoneGraphPathTestFromFragment.OutPathLinkedLanes;

	//TMap<int, FZoneGraphLinkedLane> OutPathLinkedLanes;
	

	if (CachedLinkedLanes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Path Completed"));
		return EStateTreeRunStatus::Succeeded;
	}


	if (!LaneLocation.LaneHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid lane location."));
		return EStateTreeRunStatus::Failed;
	}
	
	const FZoneGraphStorage* ZoneGraphStorage = ZoneGraphSubsystem.GetZoneGraphStorage(LaneLocation.LaneHandle.DataHandle);
	if (!ZoneGraphStorage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing ZoneGraph Storage for current lane %s."), *LaneLocation.LaneHandle.ToString());
		return EStateTreeRunStatus::Failed;
	}


	const float MoveDistance = GetDefault<UMassCrowdSettings>()->GetMoveDistance();

	// Follow Path
	FMassZoneGraphTargetLocation& WanderTargetLocation = Context.GetInstanceData(FollowPathTargetLocationHandle);

	WanderTargetLocation.LaneHandle = LaneLocation.LaneHandle;
	WanderTargetLocation.TargetDistance = LaneLocation.DistanceAlongLane + MoveDistance;
	WanderTargetLocation.NextExitLinkType = EZoneLaneLinkType::None;
	WanderTargetLocation.NextLaneHandle.Reset();
	WanderTargetLocation.bMoveReverse = false;
	WanderTargetLocation.EndOfPathIntent = EMassMovementAction::Move;


	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;

	// TODO: Make the lane caching a TMap<int(lane index), FZoneGraphLinkedLane> so that this nested for can be replaced with o1
	FZoneGraphLinkedLane CurrentPathLaneLink;// = CachedLinkedLanes[0];
	bool bNextLaneFound = false;

	if (CachedLinkedLanes.Contains(LaneLocation.LaneHandle.Index))
	{
		CurrentPathLaneLink = CachedLinkedLanes[LaneLocation.LaneHandle.Index];

		// If not on the last repeating index
		if (LaneLocation.LaneHandle.Index != CurrentPathLaneLink.DestLane.Index)
		{
			bNextLaneFound = true;
		}
	}


	// When close to end of a lane, choose next lane.
	if (WanderTargetLocation.TargetDistance > LaneLocation.LaneLength)
	{
		WanderTargetLocation.TargetDistance = FMath::Min(WanderTargetLocation.TargetDistance, LaneLocation.LaneLength);

		// Set to next type
		WanderTargetLocation.NextExitLinkType = CurrentPathLaneLink.Type;

		// If failed to find a path position
		if (!bNextLaneFound)
		{
			// Could not find next lane, fail.
			WanderTargetLocation.Reset();
			Status = EStateTreeRunStatus::Failed;

			// Set the cache to be emptied
			ZoneGraphPathTestFromFragment.CurrentlyCachedMovement = false;
			ZoneGraphPathTestFromFragment.OutPathLinkedLanes.Empty();
		}
		else
		{
			WanderTargetLocation.NextLaneHandle = CurrentPathLaneLink.DestLane;
		}
	}
	
	
	return Status;
}