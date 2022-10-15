// Fill out your copyright notice in the Description page of Project Settings.


#include "MSObserverProcessor.h"
#include "MassCommonFragments.h"

#include "MassObserverRegistry.h"
#include "Common/Misc/MSBPFunctionLibrary.h"

UMSObserverProcessor::UMSObserverProcessor()
{
	bAutoRegisterWithProcessingPhases = false;
	bRequiresGameThreadExecution = true;
}


void UMSObserverProcessor::ConfigureQueries()
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



void UMSObserverProcessor::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{

	TArray<const UScriptStruct*> ReuiredAllFragmentTypes;
	EntityQuery.GetRequiredAllFragments().ExportTypes(ReuiredAllFragmentTypes);

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
	{
		TArray<TArrayView<FMassFragment>> FragmentViews;
		for (auto Fragment : ReuiredAllFragmentTypes )
		{
			FragmentViews.Add(Context.GetMutableFragmentView(Fragment));
		}
		
		const int32 QueryLength = Context.GetNumEntities();

		for (int32 i = 0; i < QueryLength; ++i)
		{
			for (auto FragmentView : FragmentViews)
			{
				
			}
			BPExecute(FEntityHandleWrapper{Context.GetEntity(i)}, EntitySubsystem.GetWorld());

		}
	});
}


