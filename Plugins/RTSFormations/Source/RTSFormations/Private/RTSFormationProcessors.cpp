#include "RTSFormationProcessors.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

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
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();

		// Signal affected units/entities at the end
		TArray<int> UnitSignals;
		UnitSignals.Reserve(FormationSubsystem->Units.Num());

		// Since we can have multiple units, reserving is only done in the formation subsystem
		// This is because it might be possible that a batch of spawned entities should go to different units
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];

			// If for some reason the unit hasnt been created, we should create it now
			// Unfortunately, with the nature of an array, this might cause a crash if the unit index is not next in line, need to handle this somehow
			if (!FormationSubsystem->Units.IsValidIndex(RTSFormationAgent.UnitIndex))
				FormationSubsystem->Units.AddDefaulted(1);
			
			RTSFormationAgent.EntityIndex = FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Entities.Num();
			FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Entities.Emplace(Context.GetEntity(EntityIndex));

			UnitSignals.AddUnique(RTSFormationAgent.UnitIndex);
		}
		// Signal entities in the unit that their position is updated
		// @todo only notify affected entities
		for(const int& Unit : UnitSignals)
			SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units[Unit].Entities);
	});
}

URTSFormationDestroyer::URTSFormationDestroyer()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URTSFormationDestroyer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationDestroyer::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void URTSFormationDestroyer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &EntitySubsystem](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> FormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
			
			// Remove entity from units array
			if (FormationSubsystem->Units.IsValidIndex(FormationAgent.UnitIndex))
			{
				const int32 ItemIndex = FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.IndexOfByKey(Context.GetEntity(EntityIndex));
				if (ItemIndex != INDEX_NONE)
				{
					// Since we are caching the index, we need to fix the entity index that replaces the destroyed one
					// Not sure if this is the 'correct' way to handle this, but it works for now
					if (FRTSFormationAgent* ReplacementFormationAgent = EntitySubsystem.GetFragmentDataPtr<FRTSFormationAgent>(FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.Last()))
						ReplacementFormationAgent->EntityIndex = ItemIndex;
					
					FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.RemoveAtSwap(ItemIndex, 1, true);
					
					if (!FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.IsEmpty())
						SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units[FormationAgent.UnitIndex].Entities);
				}
			}
		}
		// Shrink array and signal entities
		//FormationSubsystem->Units.Shrink();
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
			const int w = RTSFormationAgent.EntityIndex / RTSFormationSettings.FormationLength;
			const int l = RTSFormationAgent.EntityIndex % RTSFormationSettings.FormationLength;
			
			// We want the formation to be 'centered' so we need to create an offset
			const FVector CenterOffset = FVector((FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Entities.Num()/RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance, (RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance, 0.f);
			const FVector UnitPosition = FormationSubsystem->Units[RTSFormationAgent.UnitIndex].UnitPosition - CenterOffset;

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());

			FVector EntityPosition = FVector(w,l,0.f);
			EntityPosition *= RTSFormationSettings.BufferDistance;
			EntityPosition += UnitPosition;
			FVector RotateValue = EntityPosition.RotateAngleAxis(FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Angle, FVector(0.f,0.f,1.f));
			
			MoveTarget.Center = RotateValue;
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 10.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
		}
	});
}
