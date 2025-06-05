// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNavMeshMoveTask.h"

#include "MassCommonFragments.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "NavMesh/NavMeshRenderingComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSNavMeshMoveTask)


bool FMSMassFindNavMeshPathWanderTargetInRadius::Link(FStateTreeLinker& Linker)
{
		
	Linker.LinkExternalData(TransformHandle);
	return true;
}

EStateTreeRunStatus FMSMassFindNavMeshPathWanderTargetInRadius::EnterState(FStateTreeExecutionContext& Context,
                                                                           const FStateTreeTransitionResult& Transition) const
{

	auto NavSystem = Cast<UNavigationSystemV1>(Context.GetWorld()->GetNavigationSystem());
	FNavLocation NavLocation;
	const FVector Origin = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
	NavSystem->GetRandomReachablePointInRadius(Origin, Radius,NavLocation);

	FMSMassFindNavMeshPathTargetInstanceData& InstanceData = Context.GetInstanceData<FMSMassFindNavMeshPathTargetInstanceData>(*this);
	
	// For now we just stand at the end of the path
	InstanceData.MoveTargetLocation.EndOfPathIntent = EMassMovementAction::Stand;
	InstanceData.MoveTargetLocation.EndOfPathPosition = NavLocation.Location;

	return EStateTreeRunStatus::Running;
}
