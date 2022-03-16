// Fill out your copyright notice in the Description page of Project Settings.


#include "MassSampleMovementProcessor.h"
#include "MassSampleFragments.h"
#include "MassCommon/Public/MassCommonFragments.h"
#include "MassMovement/Public/MassMovementFragments.h"

UMassSampleMovementProcessor::UMassSampleMovementProcessor()
{
	//This executes on any type of game client (server, standalone, client etc).
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	//You can combine flags if you want:
	//ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);

	//Using the built-in movement processor group
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;

}

void UMassSampleMovementProcessor::ConfigureQueries()
{
	
	//Only include entities that meet the following rules:

	//must have an FMoverTag
	MovementEntityQuery.AddTagRequirement<FMoverTag>(EMassFragmentPresence::All);

	//must have an FTransformFragment and we are reading and changing it
	MovementEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	//must have an FTransformFragment and we are only reading it
	MovementEntityQuery.AddRequirement<FSampleVelocityFragment>(EMassFragmentAccess::ReadOnly);

}

void UMassSampleMovementProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	//The processor's work begins!
	//Just be aware that code that affects Mass entities in here is called when we are in processing mode.


	                        //Note that this is a lambda! If you want extra data you may need to pass something into the []
	MovementEntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [](FMassExecutionContext& Context)
	{
		//Get the length of the entities in our current ExecutionContext
		const int32 NumEntities = Context.GetNumEntities();
		
		//These are what let us read and change entity data from the query in the ForEach
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		
		//This one is readonly, so we don't need Mutable
		const TConstArrayView<FSampleVelocityFragment> VelocityList = Context.GetFragmentView<FSampleVelocityFragment>();

		
		//Loop over every entity in the current chunk and do stuff!
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransform& TransformToChange = TransformList[EntityIndex].GetMutableTransform();

			FVector VelocityToMove = VelocityList[EntityIndex].Value;

			//Multiply the amount to move by delta time from the context.
			VelocityToMove = Context.GetDeltaTimeSeconds() * VelocityToMove;

			//VelocityToMove = FVector(100.0f);
                
			TransformToChange.AddToTranslation(VelocityToMove);
			
		}

		

		
	});

	
}
