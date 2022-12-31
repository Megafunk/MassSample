// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassObserverRegistry.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "MassSignalSubsystem.h"
#include "Common/Fragments/MSFragments.h"


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
	LineTraceFromPreviousPosition.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	
	LineTraceFromPreviousPosition.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	LineTraceFromPreviousPosition.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);

	LineTraceFromPreviousPosition.RegisterWithProcessor(*this);


	RotationFollowsVelocity.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	RotationFollowsVelocity.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	RotationFollowsVelocity.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	RotationFollowsVelocity.RegisterWithProcessor(*this);



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
					// Create the previous location from our velocity
					CurrentLocation - (Velocities[i].Value * Context.GetDeltaTimeSeconds()),
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

	RotationFollowsVelocity.ForEachEntityChunk(EntityManager,Context,[&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_LineTraceFromPreviousPosition);

		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		int32 NumEntities= Context.GetNumEntities();

		FCollisionQueryParams QueryParams;

		for (int32 i = 0; i < NumEntities; ++i)
		{
			auto& Transform = Transforms[i].GetMutableTransform();
			
			Transform.SetRotation(Velocities[i].Value.Rotation().Quaternion());

		}
	});

	
	if (EntitiesToSignal.Num())
	{
		SignalSubsystem->SignalEntities(MassSample::Signals::OnHit, EntitiesToSignal);
	}
}