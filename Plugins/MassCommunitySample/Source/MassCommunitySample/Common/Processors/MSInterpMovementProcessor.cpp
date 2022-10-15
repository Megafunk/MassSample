// Fill out your copyright notice in the Description page of Project Settings.


#include "MSInterpMovementProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Common/Fragments/MSFragments.h"

UMSInterpMovementProcessor::UMSInterpMovementProcessor()
{
		ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UMSInterpMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FInterpLocationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FOriginalTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);

}

void UMSInterpMovementProcessor::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
	{
		const int32 QueryLength = Context.GetNumEntities();

		auto InterpLocations = Context.GetMutableFragmentView<FInterpLocationFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		auto OriginalTransforms = Context.GetMutableFragmentView<FOriginalTransformFragment>();

		for (int32 i = 0; i < QueryLength; ++i)
		{
			auto& InterpFragment = InterpLocations[i];
			auto& Transform = Transforms[i].GetMutableTransform();

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