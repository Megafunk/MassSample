// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileHitObserver.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MSProjectileFragments.h"
#include "Fragments/MSFragments.h"

UMSProjectileHitObserver::UMSProjectileHitObserver()
{
	ObservedType = FCollisionHitTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSProjectileHitObserver::ConfigureQueries()
{

	StopHitsQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	StopHitsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	StopHitsQuery.AddRequirement<FLineTraceFragment>(EMassFragmentAccess::ReadOnly);
	StopHitsQuery.AddTagRequirement<FCollisionHitTag>(EMassFragmentPresence::All);

	//You can always add another query for different in the same observer proecssor!
	ExtraHitQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSProjectileHitObserver::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
			StopHitsQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
			{

				auto Velocities = Context.GetMutableFragmentView<FTransformFragment>();

				auto LineTraces = Context.GetFragmentView<FLineTraceFragment>();


				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{

					Context.Defer().RemoveFragment<FMassVelocityFragment>(Context.GetEntity(EntityIndex));

					auto Transform = Velocities[EntityIndex].GetMutableTransform();

					Transform.SetTranslation(LineTraces[EntityIndex].HitResult.Location);
					
					UE_LOG( LogTemp, Warning, TEXT("%i FCollisionHitTag Observer fired on frame %i"),Context.GetEntity(EntityIndex).Index,GFrameCounter);
				}

			});

			ExtraHitQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
			{
				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					UE_LOG( LogTemp, Warning, TEXT("%i FTransformFragment Observer fired on frame %i"),Context.GetEntity(EntityIndex).Index,GFrameCounter);
				}
			});
}


