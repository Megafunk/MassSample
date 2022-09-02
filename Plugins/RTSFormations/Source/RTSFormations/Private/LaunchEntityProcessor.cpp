// Fill out your copyright notice in the Description page of Project Settings.
#include "LaunchEntityProcessor.h"

#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

//----------------------------------------------------------------------//
//  ULaunchEntityProcessor
//----------------------------------------------------------------------//
ULaunchEntityProcessor::ULaunchEntityProcessor()
{
	ObservedType = FLaunchEntityFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void ULaunchEntityProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	
}

void ULaunchEntityProcessor::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void ULaunchEntityProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &EntitySubsystem](FMassExecutionContext& Context)
	{
		TConstArrayView<FLaunchEntityFragment> LaunchEntityFragments = Context.GetFragmentView<FLaunchEntityFragment>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TArrayView<FMassForceFragment> ForceFragments = Context.GetMutableFragmentView<FMassForceFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FLaunchEntityFragment& LaunchEntityFragment = LaunchEntityFragments[EntityIndex];
			FMassMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIndex];
			FMassForceFragment& ForceFragment = ForceFragments[EntityIndex];
			const FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

			MoveTargetFragment.CreateNewAction(EMassMovementAction::Animate, *GetWorld());
			ForceFragment.Value = (TransformFragment.GetTransform().GetTranslation()-LaunchEntityFragment.Origin).GetSafeNormal()*LaunchEntityFragment.Magnitude;
			UE_LOG(LogTemp, Log, TEXT("Initialize Force: %s"), *ForceFragment.Value.ToString())
		}
	});
}

//----------------------------------------------------------------------//
//  UMoveForceProcessor
//----------------------------------------------------------------------//

void UMoveForceProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	
}

void UMoveForceProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &EntitySubsystem](FMassExecutionContext& Context)
	{
		TConstArrayView<FLaunchEntityFragment> LaunchEntityFragments = Context.GetFragmentView<FLaunchEntityFragment>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TArrayView<FMassForceFragment> ForceFragments = Context.GetMutableFragmentView<FMassForceFragment>();
		TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FLaunchEntityFragment& LaunchEntityFragment = LaunchEntityFragments[EntityIndex];
			FMassMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIndex];
			FMassForceFragment& ForceFragment = ForceFragments[EntityIndex];
			FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

			if(ForceFragment.Value.Length() > 0)
			{
				TransformFragment.GetMutableTransform().AddToTranslation(ForceFragment.Value);
				//ForceFragment.Value -= FVector(0.1f)*Context.GetDeltaTimeSeconds();
				//UE_LOG(LogTemp, Log, TEXT("Force: %f"), ForceFragment.Value.Length());
				DrawDebugSphere(GetWorld(), TransformFragment.GetTransform().GetLocation(), 40.f, 5, FColor::Red);
			}
			else
			{
				EntitySubsystem.Defer().RemoveFragment<FLaunchEntityFragment>(Context.GetEntity(EntityIndex));
				ForceFragment.Value = FVector::ZeroVector;
				MoveTargetFragment.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
			}
		}
	});
}