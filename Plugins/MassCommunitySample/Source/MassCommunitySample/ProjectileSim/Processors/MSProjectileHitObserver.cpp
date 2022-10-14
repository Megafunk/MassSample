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
}

void UMSProjectileHitObserver::ConfigureQueries()
{

	StopHitsQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	StopHitsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	StopHitsQuery.AddRequirement<FLineTraceFragment>(EMassFragmentAccess::ReadOnly);
	StopHitsQuery.AddRequirement<FHitResultFragment>(EMassFragmentAccess::ReadOnly);
	StopHitsQuery.RegisterWithProcessor(*this);

	//You can always add another query for different in the same observer processor!
	CollisionHitEventQuery.AddTagRequirement<FFireHitEventTag>(EMassFragmentPresence::All);
	CollisionHitEventQuery.AddRequirement<FHitResultFragment>(EMassFragmentAccess::ReadOnly);
	CollisionHitEventQuery.RegisterWithProcessor(*this);

}

void UMSProjectileHitObserver::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{
	
			StopHitsQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
			{

				auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

				auto HitResults = Context.GetFragmentView<FHitResultFragment>();


				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					FTransformFragment& TransformFragment = Transforms[EntityIndex];

					auto HitLocation = HitResults[EntityIndex].HitResult.ImpactPoint;
					Transforms[EntityIndex].GetMutableTransform().SetTranslation(HitLocation);

					//todo: should probably think of a messy goofy way to stop the projectile. Good enough for now?
					Context.Defer().RemoveFragment<FMassVelocityFragment>(Context.GetEntity(EntityIndex));
					Context.Defer().RemoveFragment<FLineTraceFragment>(Context.GetEntity(EntityIndex));

					
				}

			});
	
			CollisionHitEventQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
			{

				auto HitResults = Context.GetFragmentView<FHitResultFragment>();

				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					auto Hitresult = HitResults[EntityIndex].HitResult;
					
					if(Hitresult.GetActor() && Hitresult.GetActor()->Implements<UMassProjectileHitInterface>())
					{
						IMassProjectileHitInterface::Execute_ProjectileHit(
							Hitresult.GetActor(),
							FEntityHandleWrapper{Context.GetEntity(EntityIndex)},
							Hitresult);
					}
				}
			});
}


