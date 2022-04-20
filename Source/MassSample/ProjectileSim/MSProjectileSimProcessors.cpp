// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassObserverRegistry.h"
#include "MSProjectileFragments.h"
#include "MassRepresentationTypes.h"
#include "Fragments/MSFragments.h"
#include "HAL/ThreadManager.h"


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

	MyQuery = LineTraceFromPreviousPosition;

	MyQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadOnly,EMassFragmentPresence::Optional);

	//LineTraceFromPreviousPosition.AddTagRequirement<FNotMovingTag>(EMassFragmentPresence::None);
}

void UMSProjectileSimProcessors::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{

	
	LineTraceFromPreviousPosition.ParallelForEachEntityChunk(EntitySubsystem,Context,[this](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_LineTraceFromPreviousPosition);

		
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
				
				FConstStructView HitResultConstStruct = FConstStructView::Make(FHitResultFragment(HitResult));
		
				Context.Defer().PushCommand(FCommandAddFragmentInstance(Entity, HitResultConstStruct));
			}
		}
	});




		
	MyQuery.ForEachEntityChunk(EntitySubsystem, Context, [](FMassExecutionContext& Context)
	{
		const TArrayView<FSampleColorFragment> OptionalFragmentList = Context.GetMutableFragmentView<FSampleColorFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			// An optional fragment array is present in our current chunk if the OptionalFragmentList isn't empty
			if(OptionalFragmentList.Num() > 0)
			{
				// Now that we know it is safe to do so, we can compute
;			UE_LOG( LogTemp, Error, TEXT("optional hit!"));

			}		
		}
	});


	


		
}