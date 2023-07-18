// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNavMeshProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeSubsystem.h"
#include "MSNavMeshFragments.h"

UMSNavMeshProcessors::UMSNavMeshProcessors()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UMSNavMeshProcessors::ConfigureQueries()
{
	EntityQuery.AddRequirement<FNavMeshAIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);

	// Do we need this and the qeury? I should probably look into this
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	
}

void UMSNavMeshProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToSignalPathDone;

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
	{
		const auto NavMeshAIFragmentList = Context.GetMutableFragmentView<FNavMeshAIFragment>();

		const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

		const auto MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			
			auto& NavMeshAIFragment = NavMeshAIFragmentList[i];
			auto& MoveTargetFragment = MoveTargetList[i];
			auto& VelocityFragment = VelocityList[i];
			
			const FTransform& AgentTransform = TransformList[i].GetTransform();

			FVector MoveDirection = (NavMeshAIFragment.NextPathNodePos - AgentTransform.GetLocation()).GetSafeNormal();
			
			VelocityFragment.Value = MoveDirection * MoveTargetFragment.DesiredSpeed.Get();
			
			MoveTargetFragment.DistanceToGoal = (MoveTargetFragment.Center -  AgentTransform.GetLocation()).Length();
			
			const float DistanceToNextGoal = (NavMeshAIFragment.NextPathNodePos -  AgentTransform.GetLocation()).Length();

			if(DistanceToNextGoal < 10.0f)
			{
				EntitiesToSignalPathDone.Add(Context.GetEntity(i));
			}
		}
	});


	if (EntitiesToSignalPathDone.Num() > 0)
	{
		Context.GetMutableSubsystem<UMassSignalSubsystem>()->SignalEntities(UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignalPathDone);
	}
}
