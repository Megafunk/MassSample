// Fill out your copyright notice in the Description page of Project Settings.
#include "LaunchEntityProcessor.h"

#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "TimerManager.h"
#include "Engine/World.h"

//----------------------------------------------------------------------//
//  ULaunchEntityProcessor
//----------------------------------------------------------------------//

void ULaunchEntityProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FInitLaunchFragment>(EMassFragmentPresence::None);
}

void ULaunchEntityProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, LaunchEntity);
}

void ULaunchEntityProcessor::SignalEntities(FMassEntityManager& EntityManager,
                                            FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ParallelForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		TArrayView<FLaunchEntityFragment> LaunchEntityFragments = Context.GetMutableFragmentView<FLaunchEntityFragment>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FLaunchEntityFragment& LaunchEntityFragment = LaunchEntityFragments[EntityIndex];
			FMassMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIndex];
			const FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

			MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *GetWorld());
			MoveTargetFragment.Center = TransformFragment.GetTransform().GetLocation()+(TransformFragment.GetTransform().GetTranslation()-LaunchEntityFragment.Origin).GetSafeNormal()*LaunchEntityFragment.Magnitude;
			MoveTargetFragment.Center.Z = 0.f;
			MoveTargetFragment.Forward = (TransformFragment.GetTransform().GetTranslation()-LaunchEntityFragment.Origin).GetSafeNormal();
			MoveTargetFragment.DistanceToGoal = (TransformFragment.GetTransform().GetTranslation()-LaunchEntityFragment.Origin).Length();
			//DrawDebugPoint(GetWorld(), MoveTargetFragment.Center+(FVector::UpVector*200.f), 40.f, FColor::Green, false, 10.f);
			//UE_LOG(LogTemp, Error, TEXT("MoveTarget: "), *MoveTargetFragment.Center.ToString());
			Context.Defer().AddTag<FInitLaunchFragment>(Context.GetEntity(EntityIndex));
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
	EntityQuery.AddTagRequirement<FInitLaunchFragment>(EMassFragmentPresence::All);
	
}

void UMoveForceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
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
			
			MoveTargetFragment.DistanceToGoal = (TransformFragment.GetTransform().GetTranslation()-MoveTargetFragment.Center).Length();

			if(MoveTargetFragment.DistanceToGoal < 50.f)
			{
				if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move)
				{
					//Context.Defer().RemoveFragment<FLaunchEntityFragment>(Context.GetEntity(EntityIndex));
					//Context.Defer().RemoveTag<FInitLaunchFragment>(Context.GetEntity(EntityIndex));
					Context.Defer().DestroyEntity(Context.GetEntity(EntityIndex));
					MoveTargetFragment.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
				}
			}
			else
			{
				DrawDebugSphere(GetWorld(), TransformFragment.GetTransform().GetLocation(), 40.f, 5, FColor::Red);
			}
		}
	});
}
