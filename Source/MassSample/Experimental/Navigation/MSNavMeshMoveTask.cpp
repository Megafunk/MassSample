// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNavMeshMoveTask.h"

#include "AITypes.h"
#include "MassStateTreeExecutionContext.h"

bool FMassNavMeshPathFollowTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(AgentRadiusHandle);
	Linker.LinkExternalData(MovementParamsHandle);
	Linker.LinkExternalData(MSSubsystemHandle);
	
	Linker.LinkInstanceDataProperty(TargetLocationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassNavMeshPathFollowTaskInstanceData, TargetLocation));
	Linker.LinkInstanceDataProperty(MovementStyleHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassNavMeshPathFollowTaskInstanceData, MovementStyle));
	Linker.LinkInstanceDataProperty(SpeedScaleHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassNavMeshPathFollowTaskInstanceData, SpeedScale));
	
	return true;
}


EStateTreeRunStatus FMassNavMeshPathFollowTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	// Do not reset of the state if current state is still active after transition, unless transitioned specifically to this state.
	if (ChangeType == EStateTreeStateChangeType::Sustained && Transition.Current != Transition.Next)
	{
		return EStateTreeRunStatus::Running;
	}
	
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	

	
	// if (!RequestPath(MassContext, TargetLocation))
	// {
	// 	return EStateTreeRunStatus::Failed;
	// }

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassNavMeshPathFollowTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	
	const UMSSubsystem& MSSubsystem = Context.GetExternalData(MSSubsystemHandle);
	const FAgentRadiusFragment& AgentRadius = Context.GetExternalData(AgentRadiusHandle);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	
	const FMassMovementStyleRef MovementStyle = Context.GetInstanceData(MovementStyleHandle);
	const FMassMovementParameters& MovementParams = Context.GetExternalData(MovementParamsHandle);

	const FVector AgentNavLocation = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
	const FVector TargetLocation = Context.GetInstanceData(TargetLocationHandle);
	const float SpeedScale = Context.GetInstanceData(SpeedScaleHandle);

	auto NavMeshSubsystem = Cast<UNavigationSystemV1>(Context.GetWorld()->GetNavigationSystem());

	

	
	//todo-performace: surely we do most of this work once per shared fragment or something? 
	const FNavAgentProperties& NavAgentProperties = FNavAgentProperties(AgentRadius.Radius);
	if (const ANavigationData* NavData = NavMeshSubsystem->GetNavDataForProps(NavAgentProperties, AgentNavLocation))
	{
		FPathFindingQuery Query(NavMeshSubsystem, *NavData, AgentNavLocation, TargetLocation);
		
		if (Query.NavData.IsValid() == false)
		{
			Query.NavData = NavMeshSubsystem->GetNavDataForProps(NavAgentProperties, Query.StartLocation);
		}

		FPathFindingResult Result(ENavigationQueryResult::Error);
		if (Query.NavData.IsValid())
		{
			//todo-performace: hierarchal paths useful or what? 
			Result = Query.NavData->FindPath(NavAgentProperties, Query);
		}

		if (Result.IsSuccessful())
		{

			const float DesiredSpeed = FMath::Min(MovementParams.GenerateDesiredSpeed(MovementStyle, MassContext.GetEntity().Index) * SpeedScale, MovementParams.MaxSpeed);

			const auto PathEndLocation = Result.Path.Get()->GetEndLocation();
			
			UE_VLOG_CIRCLE(&MSSubsystem, LogMassNavigation, Log, MoveTarget.Center, FVector::UpVector, AgentRadius.Radius, FColor::Green, TEXT_EMPTY);
			MoveTarget.DesiredSpeed.Set(DesiredSpeed);
			MoveTarget.DistanceToGoal = (PathEndLocation -  AgentNavLocation).Length();
			MoveTarget.Center = Result.Path.Get()->GetEndLocation();
			MoveTarget.CreateNewAction(EMassMovementAction::Move,*Context.GetWorld());
			
			//PFollowComp->RequestMove(FAIMoveRequest(GoalLocation), Result.Path);
		}
		// else if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
		// {
		// 	//PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
		// }
	}

	
	//return ShortPath.IsDone() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;

	return EStateTreeRunStatus::Running;
}
