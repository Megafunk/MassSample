// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassObserverRegistry.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "MassSignalSubsystem.h"
#include "MSSubsystem.h"
#include "Common/Fragments/MSFragments.h"
#include "Common/Processors/MSOctreeProcessors.h"
#include "Math/GenericOctree.h"



UMSProjectileSimLineTrace::UMSProjectileSimLineTrace()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMSProjectileSimLineTrace::ConfigureQueries()
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

	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSProjectileSimLineTrace::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesThatHitSomething;
	TArray<FMassEntityHandle> EntitiesThatWereHit;
	
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
				
				EntitiesThatHitSomething.Add(Entity);
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

	
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>(EntityManager.GetWorld());

	if (EntitiesThatHitSomething.Num())
	{
		SignalSubsystem.SignalEntities(MassSample::Signals::OnProjectileHitSomething, EntitiesThatHitSomething);
	}
	if (EntitiesThatWereHit.Num())
	{
		SignalSubsystem.SignalEntities(MassSample::Signals::OnGetHit, EntitiesThatWereHit);
	}
}


UMSProjectileSimOctreeQueryProcessor::UMSProjectileSimOctreeQueryProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteAfter.Add(UMSOctreeProcessor::StaticClass()->GetFName());
}

void UMSProjectileSimOctreeQueryProcessor::ConfigureQueries()
{
	ProjectileOctreeQuery.AddRequirement<FMSCollisionIgnoredActorsFragment>(EMassFragmentAccess::ReadWrite);
	ProjectileOctreeQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	ProjectileOctreeQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	ProjectileOctreeQuery.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	ProjectileOctreeQuery.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);

	ProjectileOctreeQuery.RegisterWithProcessor(*this);

	
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);

}

void UMSProjectileSimOctreeQueryProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{


	TArray<FMassEntityHandle> EntitiesThatHitSomething;
	TArray<FMassEntityHandle> EntitiesThatWereHit;

	ProjectileOctreeQuery.ForEachEntityChunk(EntityManager,Context,[&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_ProjectileOctreeQuery);

		const auto MassSampleSystem = EntityManager.GetWorld()->GetSubsystem<UMSSubsystem>();
		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto Transforms = Context.GetFragmentView<FTransformFragment>();

		int32 NumEntities= Context.GetNumEntities();

		FCollisionQueryParams QueryParams;

		for (int32 i = 0; i < NumEntities; ++i)
		{

			auto Velocity = Velocities[i].Value;
			auto CurrentLocation = Transforms[i].GetTransform().GetTranslation();

			TArray<FMassEntityHandle> EntitiesFound;

			// DrawDebugBox(EntityManager.GetWorld(), CurrentLocation, Velocity*Context.GetDeltaTimeSeconds(), FColor::Red);

			MassSampleSystem->Octree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(CurrentLocation, Velocity*Context.GetDeltaTimeSeconds()),
			 [&](const FMSEntityOctreeElement& OctreeElement)
			 {
				 if(OctreeElement.EntityHandle.Index != Context.GetEntity(i).Index)
				 	EntitiesFound.Add(OctreeElement.EntityHandle);
				 });


				for (auto EntityHit : EntitiesThatWereHit)
				{
					//Ignore other projectiles (in an actual game of course we would have a more specific signifier
					if(FMassEntityView(EntityManager,EntityHit).HasTag<FMSProjectileTag>())
						continue;
					
					EntitiesThatWereHit.Add(EntityHit);
				
					FHitResult HitResult;

					HitResult.ImpactPoint = CurrentLocation;
					HitResult.ImpactNormal = Velocity.GetSafeNormal();
				
					HitResult.TraceStart = CurrentLocation - (Velocity * Context.GetDeltaTimeSeconds());
					HitResult.TraceEnd = CurrentLocation;
				
					Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(Context.GetEntity(i),FHitResultFragment(HitResult));
				}
			}
		});


	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>(EntityManager.GetWorld());

	if (EntitiesThatHitSomething.Num())
	{
		SignalSubsystem.SignalEntities(MassSample::Signals::OnProjectileHitSomething, EntitiesThatHitSomething);
	}
	if (EntitiesThatWereHit.Num())
	{
		SignalSubsystem.SignalEntities(MassSample::Signals::OnGetHit, EntitiesThatWereHit);
	}
}

