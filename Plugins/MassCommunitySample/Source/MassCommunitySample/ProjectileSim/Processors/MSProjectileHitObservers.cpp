// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileHitObservers.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MSProjectileSimProcessors.h"
#include "Common/Fragments/MSFragments.h"
#include "Common/Fragments/MSOctreeFragments.h"
#include "ProjectileSim/MassProjectileHitInterface.h"
#include "MassSignalSubsystem.h"
#include "Experimental/Physics/MSMassCollision.h"
#include "Experimental/Physics/MSMassPhysics.h"
#include "VisualLogger/VisualLogger.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSProjectileHitObservers)

UMSProjectileHitObservers::UMSProjectileHitObservers()
{
	ObservedType = FMSHitResultFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);

	bRequiresGameThreadExecution = true;
}

void UMSProjectileHitObservers::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	CollisionHitEventQuery.Initialize(EntityManager);
	CollisionHitEventQuery.AddTagRequirement<FMSProjectileFireHitEventTag>(EMassFragmentPresence::All);
	CollisionHitEventQuery.AddRequirement<FMSHitResultFragment>(EMassFragmentAccess::ReadOnly);
	CollisionHitEventQuery.RegisterWithProcessor(*this);

	//You can always add another query for different things in the same observer processor!
	ResolveHitsQuery.Initialize(EntityManager);
	ResolveHitsQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	ResolveHitsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	ResolveHitsQuery.AddRequirement<FMSHitResultFragment>(EMassFragmentAccess::ReadOnly);
	ResolveHitsQuery.RegisterWithProcessor(*this);
}

void UMSProjectileHitObservers::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	
	CollisionHitEventQuery.ForEachEntityChunk( Context, [&](FMassExecutionContext& Context)
	{
	
		auto HitResults = Context.GetFragmentView<FMSHitResultFragment>();
	
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			auto Hitresult = HitResults[EntityIndex].HitResult;
					
			FMassArchetypeHandle Archetype = EntityManager.GetArchetypeForEntityUnsafe(Context.GetEntity(0));
	
					
			if(Hitresult.GetActor() && Hitresult.GetActor()->Implements<UMassProjectileHitInterface>())
			{
				IMassProjectileHitInterface::Execute_ProjectileHit(
					Hitresult.GetActor(),
					FMSEntityViewBPWrapper(Archetype,Context.GetEntity(EntityIndex)),
					Hitresult);
			}


			// A temporary example of how to resolve an entity from a hit result
			FMassEntityHandle HitEntity = UMassSamplePhysicsStorage::FindEntityHandleFromHitResult(Hitresult);
			if (HitEntity.IsValid())
			{
				Context.Defer().DestroyEntities({HitEntity});
			}
		}
	});


	ResolveHitsQuery.ForEachEntityChunk( Context, [&](FMassExecutionContext& Context)
	{
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		auto Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();

		auto HitResults = Context.GetFragmentView<FMSHitResultFragment>();


		// This is kind of a weird way to handle a "switch" Is there a better way? Manually flushing seems to cause issues.
		// Perhaps make these gamethread only as they are very sparodic?
		auto numentities = Context.GetNumEntities();


		if (Context.DoesArchetypeHaveTag<FMSProjectileRicochetTag>())
		{
			for (int32 i = 0; i < numentities; ++i)
			{
				FMassEntityHandle Entity = Context.GetEntity(i);


				const FHitResult& HitResult = HitResults[i].HitResult;
				FTransform& Transform = Transforms[i].GetMutableTransform();
				FVector& Velocity = Velocities[i].Value;

				// TODO-karl this is almost certainly wrong, I have to tool around in something a bit to get a better math setup
				// Also it should be recursive at least a few times for extra bounces after the fact
				auto ReflectionLocation = FMath::GetReflectionVector((HitResult.TraceEnd - HitResult.TraceStart) * 1.1f,
				                                                     HitResult.ImpactNormal);
				Velocity = FMath::GetReflectionVector(Velocity, HitResult.ImpactNormal);

				if (HitResult.PhysMaterial.IsValid())
				{
					Velocity *= HitResult.PhysMaterial.Get()->Restitution;
				}
				else
				{
					Velocity *= 0.5f;
				}

				// If we are too slow, we may stop here (otherwise, just consume the hitresult)
				// Magic number for now
				if (Velocity.Size() < 100.0f)
				{
					Transform.SetTranslation(HitResult.ImpactPoint);
					Context.Defer().RemoveFragment<FMassVelocityFragment>(Entity);
					Context.Defer().DestroyEntity(Entity);
				}
				else
				{
					Transform = FTransform(Velocity.Rotation(), ReflectionLocation + HitResult.ImpactPoint);

					UE_VLOG_ARROW(this, LogMass, Display, HitResult.ImpactPoint, Transform.GetLocation(), FColor::Red, TEXT("Ricochet %i"),
					              Entity.Index);
				}
				Context.Defer().RemoveFragment<FMSHitResultFragment>(Entity);
			}
		}
		else
		{
			for (int32 i = 0; i < numentities; ++i)
			{
				FMassEntityHandle Entity = Context.GetEntity(i);
				const auto& HitResult = HitResults[i].HitResult;
				auto& Transform = Transforms[i].GetMutableTransform();
				Transform.SetTranslation(HitResult.ImpactPoint);
 				Context.Defer().RemoveFragment<FMassVelocityFragment>(Entity);
				Context.Defer().RemoveFragment<FMSHitResultFragment>(Entity);

			}
		}
	});
}


UMSEntityWasHitSignalProcessor::UMSEntityWasHitSignalProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UMSProjectileSimProcessors::StaticClass()->GetFName());
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UMSEntityWasHitSignalProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 
	EntityQuery.AddTagRequirement<FMSInOctreeGridTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UMSEntityWasHitSignalProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager)
{
	Super::InitializeInternal(Owner, Manager);
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());;

	SubscribeToSignal(*SignalSubsystem, MassSample::Signals::OnGetHit);
}

void UMSEntityWasHitSignalProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                                                    FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk( Context, [&,this](FMassExecutionContext& Context)
	{
		auto Transforms = Context.GetFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			auto Transform = Transforms[i].GetTransform();
			//DrawDebugSphere(EntityManager.GetWorld(), Transform.GetLocation(), 100.0f, 16, FColor::Blue, false, 5.0f, 0, 0.4f);

			//Context.Defer().DestroyEntity(Context.GetEntity(i));
		}
	});
}
