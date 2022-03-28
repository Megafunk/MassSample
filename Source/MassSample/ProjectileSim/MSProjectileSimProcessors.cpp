// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimProcessors.h"

#include "MassCommonFragments.h"
#include "MSProjectileFragments.h"
#include "MassRepresentationTypes.h"
#include "Fragments/MSFragments.h"


void UMSProjectileSimProcessors::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	
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

	//LineTraceFromPreviousPosition.AddTagRequirement<FNotMovingTag>(EMassFragmentPresence::None);

}

void UMSProjectileSimProcessors::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{

	LineTraceFromPreviousPosition.ParallelForEachEntityChunk(EntitySubsystem,Context,[this](FMassExecutionContext& Context)
	{

		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_LineTraceFromPreviousPosition);

		const auto Linetraces = Context.GetMutableFragmentView<FLineTraceFragment>();
		const auto Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const auto CurrentPositions = Context.GetFragmentView<FTransformFragment>();
			
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{

			FVector CurrentLocation = CurrentPositions[i].GetTransform().GetTranslation();
			
			FCollisionQueryParams QueryParams;
			QueryParams.bReturnPhysicalMaterial = true;
			
			//If we hit something, add a new tag that says we did so!
			if(GetWorld()->
				LineTraceSingleByChannel(
					Linetraces[i].HitResult,
					
					CurrentLocation - Velocities[i].Value,
					CurrentLocation,
					ECollisionChannel::ECC_Camera, QueryParams))
			{
				Context.Defer().AddTag<FCollisionHitTag>(Context.GetEntity(i));

				//UE_LOG( LogTemp, Error, TEXT("BulletManager hit happened on frame: %i"),GFrameCounter);
				
			}
				
		}
			
	});
		
}