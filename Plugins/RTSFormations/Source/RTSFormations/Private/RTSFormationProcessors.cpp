#include "RTSFormationProcessors.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"

//----------------------------------------------------------------------//
//  URTSFormationInitializer
//----------------------------------------------------------------------//
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

//----------------------------------------------------------------------//
//  URTSFormationDestroyer
//----------------------------------------------------------------------//
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

		// Signal affected units/entities at the end
		TArray<int> UnitSignals;
		UnitSignals.Reserve(FormationSubsystem->Units.Num());
		
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
					UnitSignals.Emplace(FormationAgent.UnitIndex);
				}
			}
		}

		// Signal affected units/entities
		for(const int& Unit : UnitSignals)
		{
			if (FormationSubsystem->Units.IsValidIndex(Unit))
			{
				//@todo add a consistent way to reference units since the index isn't reliable
				if (FormationSubsystem->Units[Unit].Entities.Num() == 0)
				{
					FormationSubsystem->Units.RemoveAtSwap(Unit);
					continue;
				}

				// Really the only time we should notify every entity in the unit is when the center point changes
				// Every other time we just have to notify the entity that is replacing the destroyed one
				FormationSubsystem->Units[Unit].Entities.Shrink();
				SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units[Unit].Entities);
			}
		}
	});
}

//----------------------------------------------------------------------//
//  URTSAgentMovement
//----------------------------------------------------------------------//
void URTSAgentMovement::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void URTSAgentMovement::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
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

//----------------------------------------------------------------------//
//  URTSFormationUpdate
//----------------------------------------------------------------------//
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

			//const int Index = FormationSubsystem->Units[RTSFormationAgent.UnitIndex].bReverseUnit ? FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Entities.Num()-1 - RTSFormationAgent.EntityIndex : RTSFormationAgent.EntityIndex;
			//const int Index = RTSFormationAgent.EntityIndex;
			// Convert UnitIndex to X/Y coords
			//const int w = Index / RTSFormationSettings.FormationLength;
			//const int l = 4-RTSFormationAgent.EntityIndex % RTSFormationSettings.FormationLength;
			
			// We want the formation to be 'centered' so we need to create an offset
			//const FVector CenterOffset = FVector(0.f, (RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance, 0.f);
			//(FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Entities.Num()/RTSFormationSettings.FormationLength/2) * RTSFormationSettings.BufferDistance

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());

			// Set entity position based on index in formation
			//FVector EntityPosition = FVector(w,l,0.f);
			//EntityPosition *= RTSFormationSettings.BufferDistance;
			//EntityPosition -= CenterOffset;

			// Rotate unit by calculated angle
			//FVector RotateValue = EntityPosition.RotateAngleAxis(FormationSubsystem->Units[RTSFormationAgent.UnitIndex].Angle, FVector(0.f,0.f,FormationSubsystem->Units[RTSFormationAgent.UnitIndex].TurnDirection));

			// Finally add the units position to the entity position
			//RotateValue += FormationSubsystem->Units[RTSFormationAgent.UnitIndex].UnitPosition;
			
			MoveTarget.Center = RTSFormationAgent.Position;
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 10.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
		}
	});
}
