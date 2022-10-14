// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNavMeshProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeSubsystem.h"
#include "MSNavMeshFragments.h"
#include "Common/Fragments/MSFragments.h"

UMSNavMeshProcessors::UMSNavMeshProcessors()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;

}

void UMSNavMeshProcessors::Initialize(UObject& Owner)
{
	SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
}

void UMSNavMeshProcessors::ConfigureQueries()
{
	EntityQuery.AddRequirement<FNavMeshAIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);

}

void UMSNavMeshProcessors::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToSignalPathDone;

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
	{
		const auto NavMeshAIFragmentList = Context.GetMutableFragmentView<FNavMeshAIFragment>();

		const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const auto ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
		const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

		const auto MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			
			auto& NavMeshAIFragment = NavMeshAIFragmentList[i];
			auto& ForceFragment = ForceList[i];
			auto& MoveTargetFragment = MoveTargetList[i];
			auto& VelocityFragment = VelocityList[i];

			

			const FTransform& AgentTransform = TransformList[i].GetTransform();

			FVector MoveDirection = (NavMeshAIFragment.NextPathNodePos - AgentTransform.GetLocation()).GetSafeNormal();
			
			VelocityFragment.Value = MoveDirection * MoveTargetFragment.DesiredSpeed.Get();
			
			MoveTargetFragment.DistanceToGoal = (MoveTargetFragment.Center -  AgentTransform.GetLocation()).Length();

			const FVector ArrowEnd = AgentTransform.GetLocation()+(MoveDirection*50.0f); 

			
			DrawDebugLine(GetWorld(),AgentTransform.GetLocation(),ArrowEnd,FColor::Red,false,0.0f,0,1.0f);


			const float DistanceToNextGoal = (NavMeshAIFragment.NextPathNodePos -  AgentTransform.GetLocation()).Length();

			if(DistanceToNextGoal < 10.0f)
			{
				EntitiesToSignalPathDone.Add(Context.GetEntity(i));
			}
		}
	});


	if (EntitiesToSignalPathDone.Num())
	{
		SignalSubsystem->SignalEntities(UE::Mass::Signals::FollowPointPathDone, EntitiesToSignalPathDone);
	}
}
