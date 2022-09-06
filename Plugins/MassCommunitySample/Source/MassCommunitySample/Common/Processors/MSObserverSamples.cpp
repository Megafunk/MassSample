// Fill out your copyright notice in the Description page of Project Settings.

#include "MSObserverSamples.h"
#include "MassCommonFragments.h"
#include "Common/Fragments/MSFragments.h"

UMSObserverOnAdd::UMSObserverOnAdd()
{
	ObservedType = FOriginalTransformFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSObserverOnAdd::ConfigureQueries()
{
	// We still make a query here. You can add other things to query for besides the observed fragments 
	EntityQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

}

void UMSObserverOnAdd::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
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


