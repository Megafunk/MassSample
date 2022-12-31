// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileHitObserver.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "ProjectileSim/MassProjectileHitInterface.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"

UMSProjectileHitObserver::UMSProjectileHitObserver()
{
	ObservedType = FHitResultFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	bRequiresGameThreadExecution = true;
}

void UMSProjectileHitObserver::ConfigureQueries()
{
	
	CollisionHitEventQuery.AddTagRequirement<FMSProjectileFireHitEventTag>(EMassFragmentPresence::All);
	CollisionHitEventQuery.AddRequirement<FHitResultFragment>(EMassFragmentAccess::ReadOnly);
	CollisionHitEventQuery.RegisterWithProcessor(*this);
	
	//You can always add another query for different things in the same observer processor!
	ResolveHitsQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	ResolveHitsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	ResolveHitsQuery.AddRequirement<FHitResultFragment>(EMassFragmentAccess::ReadOnly);
	ResolveHitsQuery.AddTagRequirement<FMSProjectileRicochetTag>(EMassFragmentPresence::Optional);
	ResolveHitsQuery.RegisterWithProcessor(*this);



}

void UMSProjectileHitObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{


			CollisionHitEventQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
			{

				auto HitResults = Context.GetFragmentView<FHitResultFragment>();

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
				}
			});


			ResolveHitsQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
			{

				auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
				auto Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();

				auto HitResults = Context.GetFragmentView<FHitResultFragment>();


				// This is kind of a weird way to handle a "switch" Is there a better way? Manually flushing seems to cause issues.
				// Perhaps make these gamethread only as they are very sparodic?


				if(Context.DoesArchetypeHaveTag<FMSProjectileRicochetTag>())
				{
					for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
					{
						Context.Defer().RemoveFragment<FHitResultFragment>(Context.GetEntity(EntityIndex));

						const auto& HitResult = HitResults[EntityIndex].HitResult;
						auto& Transform = Transforms[EntityIndex].GetMutableTransform();
						auto& Velocity = Velocities[EntityIndex].Value;

						// TODO-karl this is almost certainly wrong, I have to tool around in something a bit to get a better math setup
						// Also it should be recursive at least a few times for extra bounces after the fact
						auto ReflectionLocation = FMath::GetReflectionVector((HitResult.TraceEnd - HitResult.TraceStart)*1.1f ,HitResult.ImpactNormal);
						Velocity = FMath::GetReflectionVector(Velocity ,HitResult.ImpactNormal);

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
						if(Velocity.Size() < 100.0f)
						{
							Transform.SetTranslation(HitResult.ImpactPoint);
							Context.Defer().RemoveFragment<FMassVelocityFragment>(Context.GetEntity(EntityIndex));
						}
						else
						{
							Transform = FTransform(Velocity.Rotation(),ReflectionLocation+HitResult.ImpactPoint);
						}


						
					}
				}
				else
				{
					for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
					{
						auto& HitResult = HitResults[EntityIndex].HitResult;
						FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();
						
						Transforms[EntityIndex].GetMutableTransform().SetTranslation(HitResult.ImpactPoint);
						
						Transform.SetTranslation(HitResult.ImpactPoint);

						// todo: should probably think of a less goofy way to stop the projectile. Good enough for now?
						
						Context.Defer().RemoveFragment<FMassVelocityFragment>(Context.GetEntity(EntityIndex));
					}
				}
				
			});
}


