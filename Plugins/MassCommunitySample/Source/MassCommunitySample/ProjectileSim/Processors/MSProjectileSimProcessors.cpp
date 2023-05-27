// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonUtils.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassObserverRegistry.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "MassSignalSubsystem.h"
#include "MSSubsystem.h"
#include "Chaos/Core.h"
#include "Chaos/GeometryParticles.h"
#include "Common/Fragments/MSFragments.h"
#include "Common/Processors/MSOctreeProcessors.h"
#include "Math/GenericOctree.h"

// Does collision calc if this view has a FMSSharedStaticMesh fragment
bool DoMassCollision(FMassEntityView& View, FMassExecutionContext& Context, FVector& PrevLocation, FVector& CurrentLocation, FHitResult& OutHit)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_MassLineTraceCollisionQuery);

	if (auto SharedMesh = View.GetConstSharedFragmentDataPtr<FMSSharedStaticMesh>())
	{
		auto WorldTransform = View.GetFragmentData<FTransformFragment>().GetTransform();

		FVector Delta = CurrentLocation - PrevLocation;

		const float DeltaMag = Delta.Size();

		FVector LocalStart = WorldTransform.InverseTransformPositionNoScale(PrevLocation);
		FVector LocalDelta = WorldTransform.InverseTransformVectorNoScale(Delta);


		ChaosInterface::FRaycastHit BestHit;
		BestHit.Distance = FLT_MAX;

		for (auto& Shape : SharedMesh->GeoPointers)
		{
			Chaos::FReal Distance;

			Chaos::FVec3 LocalPosition;
			Chaos::FVec3 LocalNormal;
			int32 FaceIndex;

			if (Shape->Raycast(LocalStart, LocalDelta / DeltaMag, DeltaMag, 0, Distance, LocalPosition, LocalNormal, FaceIndex))
			{
				if (Distance < BestHit.Distance)
				{
					BestHit.Distance = Distance;
					BestHit.WorldNormal = LocalNormal;
					//will convert to world when best is chosen
					BestHit.WorldPosition = LocalPosition;
					// BestHit.Shape = Shape;
					//BestHit.Actor = Actor->GetParticle_LowLevel();
					BestHit.FaceIndex = FaceIndex;
				}
			}
		}

		if (BestHit.Distance < FLT_MAX)
		{
			OutHit.ImpactPoint = WorldTransform.TransformPositionNoScale(BestHit.WorldPosition);
			OutHit.Location = OutHit.ImpactPoint;

			OutHit.ImpactNormal = WorldTransform.TransformVectorNoScale(BestHit.WorldNormal);

			OutHit.TraceEnd = CurrentLocation;
			OutHit.TraceStart = PrevLocation;
			
			// DrawDebugCrosshairs(Context.GetEntityManagerChecked().GetWorld(), OutHit.ImpactPoint, OutHit.Normal.Rotation(), 10.0, FColor::Magenta, false, 10);
			return true;
		}
	}
	return false;
}


void UMSProjectileSimProcessors::Initialize(UObject& Owner)
{
}


UMSProjectileSimProcessors::UMSProjectileSimProcessors()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	bRequiresGameThreadExecution = true;

	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSProjectileSimProcessors::ConfigureQueries()
{
	LineTraceFromPreviousPosition.AddRequirement<FMSCollisionIgnoredActorsFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);

	LineTraceFromPreviousPosition.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddRequirement<FMSCollisionChannelFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	LineTraceFromPreviousPosition.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	LineTraceFromPreviousPosition.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);

	LineTraceFromPreviousPosition.RegisterWithProcessor(*this);

	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSProjectileSimProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesThatHitSomething;

	LineTraceFromPreviousPosition.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_LineTraceFromPreviousPosition);


		const TArrayView<FMSCollisionIgnoredActorsFragment> IgnoredActorsFragments = Context.GetMutableFragmentView<
			FMSCollisionIgnoredActorsFragment>();
		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto Transforms = Context.GetFragmentView<FTransformFragment>();
		const auto CollisionChannels = Context.GetFragmentView<FMSCollisionChannelFragment>();

		int32 NumEntities = Context.GetNumEntities();

		FCollisionQueryParams QueryParams;

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FHitResult HitResult;

			FVector CurrentLocation = Transforms[i].GetTransform().GetTranslation();
			FVector Velocity = Velocities[i].Value;

			TEnumAsByte<ECollisionChannel> CollisionChannel = CollisionChannels[i].Channel;


			QueryParams.ClearIgnoredActors();

			//todo-perf: somehow keep the query params around as this is fairly evil to do
			if (IgnoredActorsFragments.Num() > 0)
			{
				QueryParams.AddIgnoredActors(static_cast<TArray<AActor*>>(IgnoredActorsFragments[i].IgnoredActors));
			}


			if (GetWorld()->LineTraceSingleByChannel(HitResult,
			                                         // Create the previous location from our velocity
			                                         CurrentLocation - (Velocity * Context.GetDeltaTimeSeconds()), CurrentLocation, CollisionChannel,
			                                         QueryParams))
			{
				FMassEntityHandle Entity = Context.GetEntity(i);

				EntitiesThatHitSomething.Add(Entity);
				Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, FMSHitResultFragment(HitResult));
			}
		}
	});


	if (EntitiesThatHitSomething.Num() > 0)
	{
		Context.GetMutableSubsystem<UMassSignalSubsystem>()->SignalEntities(MassSample::Signals::OnProjectileHitSomething, EntitiesThatHitSomething);
	}
}


UMSProjectileOctreeQueryProcessors::UMSProjectileOctreeQueryProcessors()
{
	ExecutionOrder.ExecuteAfter.Add(UMSOctreeProcessor::StaticClass()->GetFName());
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	
	bRequiresGameThreadExecution = false;
}


void UMSProjectileOctreeQueryProcessors::Initialize(UObject& Owner)
{
	MSSubsystem = Owner.GetWorld()->GetSubsystem<UMSSubsystem>();
}

void UMSProjectileOctreeQueryProcessors::ConfigureQueries()
{
	ProjectileOctreeQuery.AddRequirement<FMSCollisionIgnoredActorsFragment>(EMassFragmentAccess::ReadWrite);
	ProjectileOctreeQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	ProjectileOctreeQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	ProjectileOctreeQuery.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	ProjectileOctreeQuery.AddTagRequirement<FMSLineTraceTag>(EMassFragmentPresence::All);
	ProjectileOctreeQuery.AddTagRequirement<FMSProjectileOctreeQueryTag>(EMassFragmentPresence::All);

	ProjectileOctreeQuery.RegisterWithProcessor(*this);


	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSProjectileOctreeQueryProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	FMSOctree2& MSOctree2 = MSSubsystem->Octree2;

	TQueue<FMassEntityHandle,EQueueMode::Mpsc> EntitiesThatWereHit;
	std::atomic<int32> EntitiesThatWereHitNum(0);

	auto World = EntityManager.GetWorld();

	TArray<FMassExecutionContext> Contexts;
	ProjectileOctreeQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_ProjectileOctreeQuery);


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
			if(EntitiesFound.Num() > 0)
			{
				EntitiesThatWereHit.Enqueue(EntitiesFound[0]);
				++EntitiesThatWereHitNum;
			}

			FMSHitResultFragment HitResultFragment;

			for (auto Found : EntitiesFound)
			{
				auto View = FMassEntityView(EntityManager, Found);
				if (DoMassCollision(View, Context, PrevLocation, CurrentLocation, HitResultFragment.HitResult))
				{
					Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(Context.GetEntity(i), HitResultFragment);
					break;
				}
			}
		}
	});

	QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_ProjectileOctreeQueryResults);

	if (!EntitiesThatWereHit.IsEmpty())
	{
		auto Entities = UE::Mass::Utils::EntityQueueToArray(EntitiesThatWereHit,EntitiesThatWereHitNum);
		Context.GetMutableSubsystem<UMassSignalSubsystem>()->SignalEntitiesDeferred(Context, MassSample::Signals::OnGetHit, Entities);

	}
}
