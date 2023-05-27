// Fill out your copyright notice in the Description page of Project Settings.


#include "MSObserverProcessor.h"
#include "MassCommonFragments.h"

#include "MassObserverRegistry.h"
#include "Common/Misc/MSBPFunctionLibrary.h"

UMSObserverProcessorBP::UMSObserverProcessorBP()
{
	bAutoRegisterWithProcessingPhases = false;
	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	
}


void UMSObserverProcessorBP::ConfigureQueries()
{
	for (auto Fragment : FragmentRequirements)
	{
		if(Fragment.IsValid())
		EntityQuery.AddRequirement(Fragment.GetScriptStruct(),EMassFragmentAccess::ReadWrite);
	}
	for (auto Tag : TagRequirements)
	{
		if(Tag.IsValid())
		EntityQuery.AddTagRequirement(*Tag.GetScriptStruct(),EMassFragmentPresence::All);
	}
	EntityQuery.RegisterWithProcessor(*this);

}



void UMSObserverProcessorBP::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{

	TArray<const UScriptStruct*> ReuiredAllFragmentTypes;
	EntityQuery.GetRequiredAllFragments().ExportTypes(ReuiredAllFragmentTypes);
	auto World = GetWorld();

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		TArray<TArrayView<FMassFragment>> FragmentViews;
		for (auto Fragment : ReuiredAllFragmentTypes )
		{
			FragmentViews.Add(Context.GetMutableFragmentView(Fragment));
		}
		
		const int32 QueryLength = Context.GetNumEntities();

		// Surely there is a less wacky way to get an archetype inside of a context? This is definitely weird.
		FMassArchetypeHandle Archetype = EntityManager.GetArchetypeForEntityUnsafe(Context.GetEntity(0));

		for (int32 i = 0; i < QueryLength; ++i)
		{
			for (auto FragmentView : FragmentViews)
			{
				
			}
			BPExecute(FMSEntityViewBPWrapper(Archetype,Context.GetEntity(i)), World);

		}
	});
}


