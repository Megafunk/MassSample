// Fill out your copyright notice in the Description page of Project Settings.

#include "MSObserverSamples.h"
#include "MassCommonFragments.h"
#include "Common/Fragments/MSFragments.h"

UMSObserverOnAdd::UMSObserverOnAdd()
{
	ObservedType = FSampleColorFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSObserverOnAdd::ConfigureQueries()
{
	// We still make a query here. You can add other things to query for besides the observed fragments 
	// EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSObserverOnAdd::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
	{
		auto Colors = Context.GetMutableFragmentView<FSampleColorFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// When a color is added, make it a random color!
			Colors[EntityIndex].Color = FColor::MakeRandomColor();			
			UE_LOG( LogTemp, Warning, TEXT("%i SampleColorFragment Observer fired on frame %i"),Context.GetEntity(EntityIndex).Index,GFrameCounter);
		}
	});
}


