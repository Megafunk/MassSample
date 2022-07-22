#include "RTSFormationProcessors.h"

#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
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
			SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units[Unit].Entities.Array());
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
				const FMassEntityHandle* ItemIndex = FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.Find(Context.GetEntity(EntityIndex));
				if (ItemIndex)
				{
					// Since we are caching the index, we need to fix the entity index that replaces the destroyed one
					// Not sure if this is the 'correct' way to handle this, but it works for now
					//if (FRTSFormationAgent* ReplacementFormationAgent = EntitySubsystem.GetFragmentDataPtr<FRTSFormationAgent>(FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.Array().Last()))
					//	ReplacementFormationAgent->EntityIndex = FormationAgent.EntityIndex;
					
					FormationSubsystem->Units[FormationAgent.UnitIndex].Entities.Remove(*ItemIndex);
					UnitSignals.AddUnique(FormationAgent.UnitIndex);
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
				FormationSubsystem->UpdateUnitPosition(FormationSubsystem->Units[Unit].UnitPosition, Unit);
				//SignalSubsystem->SignalEntities(FormationUpdated, FormationSubsystem->Units[Unit].Entities.Array());
			}
		}
	});
}

//----------------------------------------------------------------------//
//  URTSAgentMovement
//----------------------------------------------------------------------//
void URTSAgentMovement::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void URTSAgentMovement::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FMassVelocityFragment> VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();

		const FRTSFormationSettings& FormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

		URTSFormationSubsystem* FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(GetWorld());
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();
			FVector& Velocity = VelocityFragments[EntityIndex].Value;
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];

			const FUnitInfo& Unit = FormationSubsystem->Units[RTSFormationAgent.UnitIndex];
			
			// Convert UnitIndex to X/Y coords
			const int w = RTSFormationAgent.EntityIndex / Unit.FormationLength;
			const int l = RTSFormationAgent.EntityIndex % Unit.FormationLength;
						
			// We want the formation to be 'centered' so we need to create an offset
			const FVector CenterOffset = FVector((Unit.Entities.Num()/Unit.FormationLength/2) * Unit.BufferDistance, (Unit.FormationLength/2) * Unit.BufferDistance, 0.f);

			// Set entity position based on index in formation
			FVector EntityPosition = FVector(w,l,0.f);
			EntityPosition *= Unit.BufferDistance;
			EntityPosition -= CenterOffset;

			// Rotate unit by calculated angle
			FVector RotateValue = EntityPosition.RotateAngleAxis(Unit.Angle, FVector(0.f,0.f,Unit.TurnDirection));

			// Finally add the units position to the entity position
			RotateValue += Unit.InterpolatedDestination;
			
			MoveTarget.Center = RotateValue;
			
			// Update move target values
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
			
			// Once we are close enough to our goal, create stand action
			if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
			{
				//MoveTarget.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
				MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.WalkMovement, Context.GetEntity(EntityIndex).Index));
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
}

void URTSFormationUpdate::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationUpdate::SignalEntities(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Query to calculate move target for entities based on unit index
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

		const FRTSFormationSettings& FormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 10.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
			MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.RunMovement, Context.GetEntity(EntityIndex).Index));
		}
	});
}

//----------------------------------------------------------------------//
//  URTSUpdateEntityIndex
//----------------------------------------------------------------------//
void URTSUpdateEntityIndex::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	SubscribeToSignal(UpdateIndex);
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
}

void URTSUpdateEntityIndex::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSUpdateEntityIndex::SignalEntities(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Update entity index so that they go to the closest possible position
	// Entities are signaled in order of distance to destination, this allows the NewPosition array to be sorted once
	// and cut down on iterations significantly
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
			
			// Get first index since it is sorted
			TPair<int, FVector> ClosestPos;
			float ClosestDistance = -1;
			int i=0;
			for(const TPair<int, FVector>& NewPos : FormationSubsystem->Units[FormationAgent.UnitIndex].NewPositions)
			{
				float Dist = FVector::DistSquared2D(NewPos.Value, Location);
				if (ClosestDistance == -1 || Dist < ClosestDistance)
				{
					ClosestPos = NewPos;
					ClosestDistance = Dist;
					
					// While its not perfect, this adds a hard cap to how many positions to check
					if (++i > FormationSubsystem->Units[FormationAgent.UnitIndex].FormationLength*2)
						break;
				}
			}

			// Basically scoot up entities if there is space in the front
			int& Index = ClosestPos.Key;
			
			FormationAgent.EntityIndex = Index;
			FormationSubsystem->Units[FormationAgent.UnitIndex].NewPositions.Remove(Index);

			// Call subsystem function to get entities to move
			if (FormationSubsystem->Units[FormationAgent.UnitIndex].NewPositions.Num() == 0)
			{
				FormationSubsystem->MoveEntities(FormationAgent.UnitIndex);
			}
		}
	});
}
