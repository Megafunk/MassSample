﻿#include "RTSFormationProcessors.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"

URTSFormationInitializer::URTSFormationInitializer()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSFormationInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
}

void URTSFormationInitializer::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void URTSFormationInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	// First query is to give all units an appropriate unit index.
	// @todo The unit index is given 'randomly' regardless of distance to closest formation position.
	// Ideas: When a unit is already assigned an index, we shouldnt have to recalculate it unless a unit was destroyed and the index destroyed is less than
	// the units index - pretty much like an array, would it be inefficient though?
	// Another issue is that the center offset changes when enough units are spawned, causing the positions of the other units to be off

	// Since I really want this example to be straightforward, Im going to streamline adding/remove entities so that the formation gets recalculated
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();

		// Reserve units in advance to prevent resizing array every time
		FormationSubsystem->Units.Reserve(FormationSubsystem->Units.Num()+Context.GetNumEntities());
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			RTSFormationAgent.UnitIndex = FormationSubsystem->Units.Num();
			FormationSubsystem->Units.Emplace(Context.GetEntity(EntityIndex));
		}
		
		SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units);
	});
}

URTSFormationDestroyer::URTSFormationDestroyer()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URTSFormationDestroyer::ConfigureQueries()
{
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationDestroyer::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void URTSFormationDestroyer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		const FRTSFormationSettings& RTSFormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// Remove entity from units array
			// @todo using RemoveAtSwap will cause the last unit to swap with the destroyed unit, need to fix index
			const int32 ItemIndex = FormationSubsystem->Units.IndexOfByKey(Context.GetEntity(EntityIndex));
			if (ItemIndex != INDEX_NONE)
			{
				// Since we are caching the index, we need to fix the entity index that replaces the destroyed one
				FormationSubsystem->Units.RemoveAtSwap(ItemIndex, 1, false);
			}
		}
		// Shrink array and signal entities
		FormationSubsystem->Units.Shrink();
		
		if (!FormationSubsystem->Units.IsEmpty())
			SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units);
	});
}

void URTSAgentMovement::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void URTSAgentMovement::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		FRTSFormationSettings& FormationSettings = Context.GetMutableSharedFragment<FRTSFormationSettings>();
		TArrayView<FMassVelocityFragment> VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();
			FVector& Velocity = VelocityFragments[EntityIndex].Value;

			// Update move target values
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

			// Once we are close enough to our goal, create stand action
			if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
			{
				MoveTarget.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
				Velocity = FVector::Zero();
			} 
		}
	});
}

void URTSFormationUpdate::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	SubscribeToSignal(FormationUpdated);

	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void URTSFormationUpdate::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationUpdate::SignalEntities(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Query to calculate move target for entities based on unit index
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const FRTSFormationSettings& RTSFormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

			// Convert UnitIndex to X/Y coords
			const int w = RTSFormationAgent.UnitIndex / RTSFormationSettings.FormationLength;
			const int l = RTSFormationAgent.UnitIndex % RTSFormationSettings.FormationLength;

			// We want the formation to be 'centered' so we need to create an offset
			const FVector CenterOffset((RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance, (FormationSubsystem->Units.Num()/RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance, 0.f);

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());
			MoveTarget.Center = FVector(w*RTSFormationSettings.BufferDistance-CenterOffset.Y,l*RTSFormationSettings.BufferDistance-CenterOffset.X,0.f);
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 10.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
		}
	});
}
