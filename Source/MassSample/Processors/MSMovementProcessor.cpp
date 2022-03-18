// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMovementProcessor.h"
#include "MassCommon/Public/MassCommonFragments.h"
#include "MassMovement/Public/MassMovementFragments.h"
#include "Fragments/MSFragments.h"

UMSMovementProcessor::UMSMovementProcessor()
{
	//This executes on any type of game client (server, standalone, client etc).
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	//You can combine flags if you want:
	//ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);

	//Using the built-in movement processor group
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UMSMovementProcessor::ConfigureQueries()
{
	//Only include entities that meet the following rules:

	//ALL must have an FMoverTag
	MovementEntityQuery.AddTagRequirement<FMoverTag>(EMassFragmentPresence::All);

	//must have an FTransformFragment and we are reading and changing it
	MovementEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	//must have an FTransformFragment and we are only reading it
	MovementEntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);


	/** FIXME: Conceptual - UMassRandomVelocityInitializer might be stealing FMassVelocityFragment,
	noticed that if I put an input matching what the UMassRandomVelocityInitializer expects
	the velocity gets randomized. Maybe we require to create a new fragment exclusive for the trait **/
	// FIXME: Revise fragment reusability. - document this docx.
}

void UMSMovementProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
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
		const TConstArrayView<FMassForceFragment> ForceList = Context.GetFragmentView<FMassForceFragment>();

		//Loop over every entity in the current chunk and do stuff!
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransform& TransformToChange = TransformList[EntityIndex].GetMutableTransform();

			FVector DeltaForce = ForceList[EntityIndex].Value;
			
			//Multiply the amount to move by delta time from the context.
			DeltaForce = Context.GetDeltaTimeSeconds() * DeltaForce;
			
			TransformToChange.AddToTranslation(DeltaForce);
		}
	});
}
