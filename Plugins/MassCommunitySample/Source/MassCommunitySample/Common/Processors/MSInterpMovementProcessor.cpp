// Fill out your copyright notice in the Description page of Project Settings.


#include "MSInterpMovementProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Common/Fragments/MSFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSInterpMovementProcessor)

UMSInterpMovementProcessor::UMSInterpMovementProcessor()
{
		ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UMSInterpMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FInterpLocationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FOriginalTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);

}

void UMSInterpMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [&,this](FMassExecutionContext& Context)
	{
		const int32 QueryLength = Context.GetNumEntities();

		TArrayView<FInterpLocationFragment> InterpLocations = Context.GetMutableFragmentView<FInterpLocationFragment>();
		TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		TConstArrayView<FOriginalTransformFragment> OriginalTransforms = Context.GetFragmentView<FOriginalTransformFragment>();

		for (int32 i = 0; i < QueryLength; ++i)
		{
			FInterpLocationFragment& InterpFragment = InterpLocations[i];
			FTransform& Transform = Transforms[i].GetMutableTransform();

			const float DeltaTime = Context.GetDeltaTimeSeconds();
			
			InterpFragment.Time = InterpFragment.Time+(DeltaTime/InterpFragment.Duration);
			
			if (InterpFragment.Time > 1.0f)
			{
				InterpFragment.bForwardDirection = !InterpFragment.bForwardDirection;
				InterpFragment.Time = FMath::Abs(InterpFragment.Time-InterpFragment.Duration);
				Swap(InterpFragment.StartingLocation,InterpFragment.TargetLocation);
			}

			auto NewLocation = FMath::Lerp<FVector>(
				InterpFragment.StartingLocation,
				InterpFragment.TargetLocation,
				InterpFragment.Time) + OriginalTransforms[i].Transform.GetLocation();
			
			Transform.SetLocation(NewLocation);
		}
	});
}
