// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassObserverRegistry.h"
#include "MSProjectileFragments.h"
#include "MassRepresentationTypes.h"
#include "Fragments/MSFragments.h"


void UMSProjectileSimProcessors::Initialize(UObject& Owner)
{
	
}


UMSProjectileSimProcessors::UMSProjectileSimProcessors()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMSProjectileSimProcessors::ConfigureQueries()
{
	LineTraceFromPreviousPosition.AddRequirement<FLineTraceFragment>(EMassFragmentAccess::ReadWrite);
	LineTraceFromPreviousPosition.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddTagRequirement<FProjectileTag>(EMassFragmentPresence::All);

	//LineTraceFromPreviousPosition.AddTagRequirement<FNotMovingTag>(EMassFragmentPresence::None);
}

void UMSProjectileSimProcessors::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{

	
	LineTraceFromPreviousPosition.ForEachEntityChunk(EntitySubsystem,Context,[this](FMassExecutionContext& Context)
	{


		const auto Linetraces = Context.GetMutableFragmentView<FLineTraceFragment>();
		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto Transforms = Context.GetFragmentView<FTransformFragment>();

		int32 NumEntities= Context.GetNumEntities();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FHitResult HitResult;
		
			FVector CurrentLocation = Transforms[i].GetTransform().GetTranslation();
			
			//If we hit something, add a new fragment with the data!
			if(GetWorld()->
				LineTraceSingleByChannel(
					HitResult,
					
					CurrentLocation - Velocities[i].Value,
					CurrentLocation,
					ECollisionChannel::ECC_Camera,
					Linetraces[i].QueryParams
				))
			{
		
				FMassEntityHandle Entity = Context.GetEntity(i);
				
		
		
				//Context.Defer().AddFragment<FHitResultFragment>(Entity);
				//todo: is this really the only way to do this cleanly? I must be missing something?
		
				//GetWorld()->GetSubsystem<UMassObserverRegistry>()
		
		
				FConstStructView HitResultConstStruct = FConstStructView::Make(FHitResultFragment(HitResult));
		
				Context.Defer().PushCommand(FCommandAddFragmentInstance(Entity, HitResultConstStruct));
		
				
			}
				
		}


			
	});
		
}