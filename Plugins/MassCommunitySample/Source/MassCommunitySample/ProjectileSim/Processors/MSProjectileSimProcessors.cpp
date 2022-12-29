// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassObserverRegistry.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "MassRepresentationTypes.h"
#include "MassSignalSubsystem.h"
#include "Common/Fragments/MSFragments.h"
#include "HAL/ThreadManager.h"


void UMSProjectileSimProcessors::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
}


UMSProjectileSimProcessors::UMSProjectileSimProcessors()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMSProjectileSimProcessors::ConfigureQueries()
{
	LineTraceFromPreviousPosition.AddRequirement<FMSCollisionIgnoredActorsFragment>(EMassFragmentAccess::ReadWrite);
	LineTraceFromPreviousPosition.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);
	LineTraceFromPreviousPosition.AddTagRequirement<FProjectileTag>(EMassFragmentPresence::All);

	LineTraceFromPreviousPosition.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.RegisterWithProcessor(*this);

	MyQuery = LineTraceFromPreviousPosition;

	MyQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadOnly,EMassFragmentPresence::Optional);
	MyQuery.RegisterWithProcessor(*this);

	//LineTraceFromPreviousPosition.AddTagRequirement<FNotMovingTag>(EMassFragmentPresence::None);
}

void UMSProjectileSimProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToSignal;

	
	LineTraceFromPreviousPosition.ForEachEntityChunk(EntityManager,Context,[&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_LineTraceFromPreviousPosition);

		
		const auto IgnoredActorsFragments = Context.GetMutableFragmentView<FMSCollisionIgnoredActorsFragment>();
		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto Transforms = Context.GetFragmentView<FTransformFragment>();

		int32 NumEntities= Context.GetNumEntities();

		FCollisionQueryParams QueryParams;

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FHitResult HitResult;

			FVector CurrentLocation = Transforms[i].GetTransform().GetTranslation();
			
			
			QueryParams.ClearIgnoredActors();
			QueryParams.AddIgnoredActors(IgnoredActorsFragments[i].IgnoredActors);
			
			//If we hit something, add a new fragment with the data!
			if(GetWorld()->
				LineTraceSingleByChannel(
					HitResult,
					
					CurrentLocation - Velocities[i].Value,
					CurrentLocation,
					ECollisionChannel::ECC_Camera,
					QueryParams
				))
			{
		
				FMassEntityHandle Entity = Context.GetEntity(i);
				
		
				Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, FHitResultFragment(HitResult));
				
				EntitiesToSignal.Add(Entity);
			}
		}
	});


	if (EntitiesToSignal.Num())
	{
		SignalSubsystem->SignalEntities(MassSample::Signals::OnHit, EntitiesToSignal);
	}

		
	MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		const TArrayView<FSampleColorFragment> OptionalFragmentList = Context.GetMutableFragmentView<FSampleColorFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			// An optional fragment array is present in our current chunk if the OptionalFragmentList isn't empty
			if(OptionalFragmentList.Num() > 0)
			{
				// Now that we know it is safe to do so, we can compute... something!
			}		
		}
	});


	


		
}