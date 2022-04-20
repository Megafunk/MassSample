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


	TMap<int, int>& CachedLinkedLanes = ZoneGraphPathTestFromFragment.OutPathLinkedLanes;
	


	if (!LaneLocation.LaneHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid lane location."));
		// Set the cache to be emptied
		ZoneGraphPathTestFromFragment.CurrentlyCachedMovement = false;
		ZoneGraphPathTestFromFragment.OutPathLinkedLanes.Empty();
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
	

	
	


	//FZoneGraphLinkedLane CurrentPathLaneLink;
	FZoneLaneLinkData CurrentPathLaneLink;
	float TargetLaneLength = LaneLocation.LaneLength;
	bool bNextLaneFound = true;
	int CurrentPathIndex = ZoneGraphPathTestFromFragment.CurrentPathIndex;



	


	if (CachedLinkedLanes.Contains(LaneLocation.LaneHandle.Index))
	{
		CurrentPathLaneLink = ZoneGraphStorage->LaneLinks[CachedLinkedLanes[LaneLocation.LaneHandle.Index]];
		WanderTargetLocation.TargetDistance = LaneLocation.DistanceAlongLane + MoveDistance;
		WanderTargetLocation.NextExitLinkType = CurrentPathLaneLink.Type;
		const FZoneGraphLaneHandle NextLaneHandle = FZoneGraphLaneHandle(CurrentPathLaneLink.DestLaneIndex, ZoneGraphStorage->DataHandle);
		WanderTargetLocation.NextLaneHandle = NextLaneHandle;
		if (WanderTargetLocation.TargetDistance >= TargetLaneLength)
		{
			WanderTargetLocation.TargetDistance = FMath::Min(WanderTargetLocation.TargetDistance, TargetLaneLength);
		}
		
	}
	else if (LaneLocation.LaneHandle.Index == ZoneGraphPathTestFromFragment.TargetLocation.LaneHandle.Index)
		//&& LaneLocation.DistanceAlongLane < ZoneGraphPathTestFromFragment.TargetLocation.DistanceAlongLane
	{
		UE_LOG(LogTemp, Warning, TEXT("Your time has come, End of the line!"));
		WanderTargetLocation.TargetDistance = FMath::Min(ZoneGraphPathTestFromFragment.TargetLocation.DistanceAlongLane, TargetLaneLength);
		WanderTargetLocation.EndOfPathIntent = EMassMovementAction::Stand;

		// TODO/NOTE: This is here because with such a setup, follow path does not provide a direct response.
		if (LaneLocation.DistanceAlongLane > ZoneGraphPathTestFromFragment.TargetLocation.DistanceAlongLane - (MoveDistance / 2))
		{
			ZoneGraphPathTestFromFragment.CurrentlyCachedMovement = false;
			ZoneGraphPathTestFromFragment.OutPathLinkedLanes.Empty();
			WanderTargetLocation.Reset();
			UE_LOG(LogTemp, Warning, TEXT("Failed or End"));
			
			return EStateTreeRunStatus::Failed;
		}
		
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		// Set the cache to be emptied
		ZoneGraphPathTestFromFragment.CurrentlyCachedMovement = false;
		ZoneGraphPathTestFromFragment.OutPathLinkedLanes.Empty();
		UE_LOG(LogTemp, Warning, TEXT("Failed or End"));
		return EStateTreeRunStatus::Failed;
	}


	UE_LOG(LogTemp, Warning, TEXT("From %s -> %s "), *WanderTargetLocation.LaneHandle.ToString(), *WanderTargetLocation.NextLaneHandle.ToString());
	
	


	return EStateTreeRunStatus::Succeeded;
}