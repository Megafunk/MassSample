// Fill out your copyright notice in the Description page of Project Settings.

#include "MSObserverSamples.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Common/Fragments/MSFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSObserverSamples)

UMSObserverOnAdd::UMSObserverOnAdd()
{
	ObservedType = FOriginalTransformFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSObserverOnAdd::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// We still make a query here. You can add other things to query for besides the observed fragments
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);

}

void UMSObserverOnAdd::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [&,this](FMassExecutionContext& Context)
	{
		auto OriginalTransforms = Context.GetMutableFragmentView<FOriginalTransformFragment>();
		auto Transforms = Context.GetFragmentView<FTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// When an original transform is added, set it to our transform!
			OriginalTransforms[EntityIndex].Transform = Transforms[EntityIndex].GetTransform();			
		}
	});
}


