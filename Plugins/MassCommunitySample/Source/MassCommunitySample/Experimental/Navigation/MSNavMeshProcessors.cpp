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

void UMSNavMeshProcessors::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FNavMeshAIFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

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
		const auto NavMeshAIFragmentList = Context.GetFragmentView<FNavMeshAIFragment>();

		const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
		const auto& MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const FNavMeshAIFragment& NavMeshAIFragment = NavMeshAIFragmentList[i];
			FMassMoveTargetFragment& MoveTargetFragment = MoveTargetList[i];
			
			const FTransform& Transform = TransformList[i].GetTransform();
			
			MoveTargetFragment.DistanceToGoal = (MoveTargetFragment.Center -  Transform.GetLocation()).Length();
			
			const float DistanceToNextGoal = (NavMeshAIFragment.NextPathNodePos -  Transform.GetLocation()).Length();

			if(DistanceToNextGoal < 100.0f)
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
