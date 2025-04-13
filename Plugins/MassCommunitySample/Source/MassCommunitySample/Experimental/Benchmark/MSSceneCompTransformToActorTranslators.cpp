// Fill out your copyright notice in the Description page of Project Settings.


#include "MSSceneCompTransformToActorTranslators.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

UMSTransformToSceneCompTranslatorFastPath::UMSTransformToSceneCompTranslatorFastPath()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	RequiredTags.Add<FMSMassTransformToSceneCompFastPathTag>();

	bRequiresGameThreadExecution = true;
}

void UMSTransformToSceneCompTranslatorFastPath::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FMassSceneComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RequireMutatingWorldAccess(); // due to mutating World. We can do part of this async maybe...
}

void UMSTransformToSceneCompTranslatorFastPath::Execute(FMassEntityManager& EntityManager,
                                                        FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const auto ComponentList = Context.GetFragmentView<FMassSceneComponentWrapperFragment>();
		const auto LocationList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();


		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (USceneComponent* RootComponent = ComponentList[i].Component.Get())
			{
				SetWorldTransformFastPath(RootComponent, LocationList[i].GetTransform());
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
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
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
