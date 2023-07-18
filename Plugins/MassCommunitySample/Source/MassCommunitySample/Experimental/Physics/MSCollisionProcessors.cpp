// Fill out your copyright notice in the Description page of Project Settings.


#include "MSCollisionProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassCommonUtils.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"
#include "MSMassCollision.h"

#include "Common/Fragments/MSFragments.h"
#include "Common/Processors/MSOctreeProcessors.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"


UMSEntityCollisionQueryProcessors::UMSEntityCollisionQueryProcessors()
{
	ExecutionOrder.ExecuteAfter.Add(UMSOctreeProcessor::StaticClass()->GetFName());
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}


void UMSEntityCollisionQueryProcessors::Initialize(UObject& Owner)
{
	MSSubsystem = Owner.GetWorld()->GetSubsystem<UMSSubsystem>();
}

void UMSEntityCollisionQueryProcessors::ConfigureQueries()
{
	OctreeQueryQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	OctreeQueryQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	OctreeQueryQuery.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);
	OctreeQueryQuery.AddTagRequirement<FMSOctreeQueryTag>(EMassFragmentPresence::All);
	OctreeQueryQuery.RegisterWithProcessor(*this);
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSEntityCollisionQueryProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	FMSOctree2& MSOctree2 = MSSubsystem->Octree2;

	// Other entities we hit
	TQueue<FMassEntityHandle, EQueueMode::Mpsc> EntitiesCollided;
	std::atomic<int32> EntitiesThatWereHitNum(0);
	
	TArray<FMassExecutionContext> Contexts;
	OctreeQueryQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASSSAMPLE_OctreeQueryQuery);


		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto Transforms = Context.GetFragmentView<FTransformFragment>();

		int32 NumEntities = Context.GetNumEntities();
		for (int32 i = 0; i < NumEntities; ++i)
		{
			auto Velocity = Velocities[i].Value;
			auto CurrentLocation = Transforms[i].GetTransform().GetLocation();

			TArray<FMassEntityHandle> EntitiesFound;
			FVector DistanceTraveled = (Velocity * Context.GetDeltaTimeSeconds());

			// We do this enough to make it seem worthwhile to just make a PrevLocation fragment?
			FVector PrevLocation = CurrentLocation - DistanceTraveled;

			// Extents are half sized
			auto QueryBounds = FBoxCenterAndExtent(CurrentLocation - (DistanceTraveled / 2), (DistanceTraveled.GetAbs() / 2));

			// A smarter person would know a fancier way to skip extra checks here but I am okay with "good enough"
			// Spatial datastructures should ideally be used with an access pattern in mind but I wanted something that just.. works
			MSOctree2.FindElementsWithBoundsTest(QueryBounds, [&](const FMSEntityOctreeElement& Element)
			{
				if (FMath::LineBoxIntersection(Element.Bounds.GetBox(), PrevLocation, CurrentLocation, CurrentLocation - PrevLocation) && Element.
					EntityHandle.Index != Context.GetEntity(i).Index)
				{
					EntitiesFound.Add(Element.EntityHandle);
				}
			});


			// todo: probably want an actual event here. Adding a hit result fragment is not exactly expensive but still feels wrong
			for (auto EntityFound : EntitiesFound)
			{
				EntitiesCollided.Enqueue(EntityFound);
				++EntitiesThatWereHitNum;


				// Snipping this for now as it appears to not work well... I think we need to cache implicit objects after all
				// auto View = FMassEntityView(EntityManager, EntityFound);
				//
				// // if we can hit it with our custom collision setup, send a hit result fragment to the current processing entity
				// FHitResult HitResult;
				// if (MassSample::Collision::SingleRaycastEntityView(View, PrevLocation, CurrentLocation, HitResult))
				// {
				// 	// be aware, we currently shove the hit result fragment on the entity that DID the hit, not the one being hit!
				// 	FMassEntityHandle Entity = Context.GetEntity(i);
				// 	Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, FMSHitResultFragment(HitResult));
				// }
			}
		}
	});

	QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_ProjectileOctreeQueryResults);


	if (EntitiesThatWereHitNum > 0)
	{
		TArray<FMassEntityHandle> Entities = UE::Mass::Utils::EntityQueueToArray(EntitiesCollided, EntitiesThatWereHitNum);
		Context.GetMutableSubsystem<UMassSignalSubsystem>()->SignalEntities(MassSample::Signals::OnEntityHitSomething, Entities);
	}
}
