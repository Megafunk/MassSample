// Fill out your copyright notice in the Description page of Project Settings.


#include "MSSceneCompTransformToActorTranslators.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSSceneCompTransformToActorTranslators)

UMSTransformToSceneCompTranslatorFastPath::UMSTransformToSceneCompTranslatorFastPath()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	RequiredTags.Add<FMSMassTransformToSceneCompFastPathTag>();

	// Arguably some of this could be done on another thread but that might be more complicated
	bRequiresGameThreadExecution = true;
}

void UMSTransformToSceneCompTranslatorFastPath::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ParallelStepQuery.Initialize(EntityManager);
	AddRequiredTagsToQuery(ParallelStepQuery);
	ParallelStepQuery.AddRequirement<FMassSceneComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	ParallelStepQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	ParallelStepQuery.RequireMutatingWorldAccess(); // due to mutating World. We can do part of this async maybe...
	ParallelStepQuery.RegisterWithProcessor(*this);

	// This query is largely the same so it can just copy the other's requirements
	MainThreadStepQuery.Initialize(EntityManager);
	MainThreadStepQuery = ParallelStepQuery;
	MainThreadStepQuery.RegisterWithProcessor(*this);
}

void UMSTransformToSceneCompTranslatorFastPath::Execute(FMassEntityManager& EntityManager,
                                                        FMassExecutionContext& Context)
{

	using namespace Chaos;
	
	// Parallel work that doesn't involve writing external things (Usually... It depends on the scene component's bounds impl though)
	ParallelStepQuery.ParallelForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const auto& ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (USceneComponent* RootComponent = ComponentList[i].Component.Get())
			{
				MassSample::Actors::SetWorldTransformFastPathParallelStep(RootComponent, TransformList[i].GetTransform());
			}
		}
	});
	
	MassSample::Actors::FMassSampleAABBQueue AccelerationQueue;
	
	MainThreadStepQuery.ParallelForEachEntityChunk( Context, [this, &AccelerationQueue](FMassExecutionContext& Context)
	{
		const auto& ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();


		FPhysScene* PhysicsScene = Context.GetWorld()->GetPhysicsScene();
		
		FPhysicsCommand::ExecuteRead(PhysicsScene, [&]()
		{
			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (USceneComponent* RootComponent = ComponentList[i].Component.Get())
				{
					MassSample::Actors::SetPhysicsBodyTransformBatchedWhilePhysicsLocked(RootComponent, TransformList[i].GetTransform(),AccelerationQueue);
				}
			}
		});
	});

	FPhysScene* PhysicsScene = Context.GetWorld()->GetPhysicsScene();

	FPhysicsCommand::ExecuteWrite(PhysicsScene, [&]()
	{
		auto SpacialAccel = PhysicsScene->GetSpacialAcceleration();
		auto Solver = PhysicsScene->GetSolver();

		if (SpacialAccel && Solver)
		{
				TPair<FPhysicsActorHandle,Chaos::FAABB3> AABBElement;
			    while (AccelerationQueue.Dequeue(AABBElement))
			    {
				    FPhysicsActorHandle& Actor = AABBElement.Key;


			    	const FRigidBodyHandle_External& Body_External = Actor->GetGameThreadAPI();
			    	
					FAccelerationStructureHandle AccelerationHandle(Actor->GetParticle_LowLevel());
			    	
				    const FAABB3& WorldBounds = AABBElement.Value;
			    	
			        SpacialAccel->UpdateElementIn(AccelerationHandle, WorldBounds, true, Body_External.SpatialIdx());
			        Solver->UpdateParticleInAccelerationStructure_External(Actor->GetParticle_LowLevel(), EPendingSpatialDataOperation::Update);
			    }
		}
	});
	
	// Main thread work
	MainThreadStepQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const auto& ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();


		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (USceneComponent* RootComponent = ComponentList[i].Component.Get())
			{
				MassSample::Actors::SetWorldTransformFastPathMainThreadStep(RootComponent, TransformList[i].GetTransform());
			}
		}
	});
	
}

UMSSceneCompTransformToMassTranslator::UMSSceneCompTransformToMassTranslator() : EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	RequiredTags.Add<FMSSceneComponentTransformToMassTag>();
}

void UMSSceneCompTransformToMassTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FMassSceneComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSSceneCompTransformToMassTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const auto ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const auto LocationList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (const USceneComponent* AsComponent = ComponentList[i].Component.Get())
			{
				LocationList[i].GetMutableTransform() = AsComponent->GetComponentTransform();
			}
		}
	});
}
