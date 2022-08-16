// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "DrawDebugHelpers.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationProcessors.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void URTSFormationSubsystem::DestroyEntity(UMassAgentComponent* Entity)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySubsystem);
	
	EntitySubsystem->Defer().DestroyEntity(Entity->GetEntityHandle());
}

void URTSFormationSubsystem::UpdateUnitPosition(const FVector& NewPosition, int UnitIndex)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UpdateUnitPosition"))
	if (!ensure(Units.IsValidIndex(UnitIndex))) { return; }

	// Convenience variables
	FUnitInfo& Unit = Units[UnitIndex];
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	TMap<int, FVector>& NewPositions = Unit.NewPositions;

	// Empty NewPositions Map to make room for new calculations
	NewPositions.Empty();
	NewPositions.Reserve(Unit.Entities.Num());

	// Calculate entity positions for new destination
	// This is the logic that can change formation types
	const FVector CenterOffset = FVector((Unit.Entities.Num()/Unit.FormationLength/2) * Unit.BufferDistance, (Unit.FormationLength/2) * Unit.BufferDistance, 0.f);
	int PlacedUnits = 0;
	int PosIndex = 0;
	while (PlacedUnits < Unit.Entities.Num())
	{
		float w = PosIndex / Unit.FormationLength;
		float l = PosIndex % Unit.FormationLength;

		// Hollow formation logic (2 layers)
		if (Unit.bHollow && Unit.Formation == Rectangle)
		{
			int Switch = Unit.Entities.Num() - Unit.FormationLength*2;
			if (w != 0 && w != 1 && !(PlacedUnits >= Switch)
				&& l != 0 && l != 1 && l != Unit.FormationLength-1 && l != Unit.FormationLength-2)
			{
				PosIndex++;
				continue;
			}
		}

		// Circle formation
		if (Unit.Formation == Circle)
		{
			int AmountPerRing = Unit.Entities.Num() / Unit.Rings;
			float Angle = PosIndex * PI * 2 / AmountPerRing;
			float Radius = Unit.FormationLength + (PosIndex / AmountPerRing * 1.5f);
			w = FMath::Cos(Angle) * Radius;
			l = FMath::Sin(Angle) * Radius;
		}
		
		PlacedUnits++;
		FVector Position = FVector(w,l,0.f);
		Position *= Unit.BufferDistance;
		if (Unit.Formation == Rectangle)
			Position -= CenterOffset;
		if (!Unit.bBlendAngle)
			Position = Position.RotateAngleAxis(Unit.InterpolatedAngle, FVector(0.f,0.f,Unit.TurnDirection));
		//Position += NewPosition;
		NewPositions.Emplace(PosIndex, Position);

		DrawDebugPoint(GetWorld(), Position, 20.f, FColor::Red, false, 10.f);
		PosIndex++;
	}

	// The position to order entities/positions is based on the furthest destination location
	Unit.FarCorner = NewPosition;

	NewPositions.ValueSort([&Unit, &NewPosition](const FVector& A, const FVector& B)
	{
		return FVector::DistSquared2D(A+NewPosition, Unit.InterpolatedDestination) < FVector::DistSquared2D(B+NewPosition, Unit.InterpolatedDestination);
	});
	
	if (NewPositions.Num())
	{
		TArray<FVector> NewArray;
		NewArray.Reserve(NewPositions.Num());
		NewPositions.GenerateValueArray(NewArray);
		Unit.FarCorner = NewArray[0];
	}
	DrawDebugPoint(GetWorld(), Unit.FarCorner, 30.f, FColor::Green, false, 10.f);
	
	Unit.Entities.Sort([&EntitySubsystem, &Unit](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		//@todo Find if theres a way to move this logic to a processor, most of the cost is coming from retrieving the location
		const FVector& LocA = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(A).Offset;
		const FVector& LocB = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(B).Offset;
		return FVector::DistSquared2D(LocA, Unit.FarCorner) > FVector::DistSquared2D(LocB, Unit.FarCorner);
	});
	
	NewPositions.ValueSort([&Unit](const FVector& A, const FVector& B)
	{
		return FVector::DistSquared2D(A, Unit.FarCorner) > FVector::DistSquared2D(B, Unit.FarCorner);
	});

	// Update the entities new index gradually using signals. This allows the bulk of processing to be spread out between frames
	FMassExecutionContext Context = EntitySubsystem->CreateExecutionContext(GetWorld()->GetDeltaSeconds());
	TArray<FMassEntityHandle> Entities = Unit.Entities.Array();
	for(int i=0;i<Unit.Entities.Num();++i)
	{
		GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntity(UpdateIndex, Entities[i], 0.01*(i/Unit.FormationLength));
	}
}

void URTSFormationSubsystem::MoveEntities(int UnitIndex)
{
	FUnitInfo& Unit = Units[UnitIndex];
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	
	// Final sort to ensure that entities are signaled from front to back
	Unit.Entities.Sort([&EntitySubsystem, &Unit](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		// Find if theres a way to move this logic to a processor, most of the cost is coming from retrieving the location
		const FVector& LocA = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(A).Offset;
		const FVector& LocB = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(B).Offset;
		return FVector::DistSquared2D(LocA, Unit.FarCorner) < FVector::DistSquared2D(LocB, Unit.FarCorner);
	});
	
	CurrentIndex = 0;

	// Signal entities to begin moving
	FMassExecutionContext Context = EntitySubsystem->CreateExecutionContext(GetWorld()->GetDeltaSeconds());
	TArray<FMassEntityHandle> Entities = Unit.Entities.Array();
	for(int i=0;i<Unit.Entities.Num();++i)
	{
		GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntity(FormationUpdated, Entities[i], 0.1*(i/Unit.FormationLength));
	} 
}

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, int UnitIndex)
{
	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 5.f, 0, 25.f);

	FUnitInfo& Unit = Units[UnitIndex];
	// Calculate turn direction and angle for entities in unit
	Unit.TurnDirection = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal().Y > 0 ? 1.f : -1.f;
	FVector OldDir = Unit.ForwardDir;
	Unit.ForwardDir = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal();
	Unit.Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal(),FVector::ForwardVector)));

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	for(const FMassEntityHandle& Entity : Unit.Entities)
	{
		EntitySubsystem->GetFragmentDataPtr<FMassMoveTargetFragment>(Entity)->CreateNewAction(EMassMovementAction::Stand, *GetWorld());
		EntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(Entity)->Value = FVector::Zero();
	}
	
	Unit.UnitPosition = NewPosition;

	// Instantly set the angle since entity indexes will change
	Unit.InterpolatedAngle = OldDir.Dot(Unit.ForwardDir) > 0.25 ? Unit.InterpolatedAngle : Unit.Angle;
	Unit.bBlendAngle = OldDir.Dot(Unit.ForwardDir) > 0.25;
	UE_LOG(LogTemp, Error, TEXT("Dot: %f"), OldDir.Dot(Unit.ForwardDir));
	
	
	UpdateUnitPosition(NewPosition, UnitIndex);
}

void URTSFormationSubsystem::SpawnEntitiesForUnit(int UnitIndex, const UMassEntityConfigAsset* EntityConfig, int Count)
{
	if (!ensure(Units.IsValidIndex(UnitIndex))) { return; }

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	// Reserve space for the new units, the space will be filled in a processor
	Units[UnitIndex].Entities.Reserve(Units[UnitIndex].Entities.Num()+Count);
	
	TArray<FMassEntityHandle> Entities;
	const FMassEntityTemplate* EntityTemplate = EntityConfig->GetConfig().GetOrCreateEntityTemplate(*UGameplayStatics::GetPlayerPawn(this, 0), *EntityConfig);

	// We are doing a little bit of work here since we are setting the unit index manually
	// Otherwise, using SpawnEntities would be perfectly fine
	// @todo find if there is a better way to modify templates in code
	TArray<FMassEntityHandle> SpawnedEntities;
	TSharedRef<UMassEntitySubsystem::FEntityCreationContext> CreationContext = EntitySubsystem->BatchCreateEntities(EntityTemplate->GetArchetype(), Count, SpawnedEntities);

	// Set the template default values for the entities
	TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate->GetInitialFragmentValues();
	EntitySubsystem->BatchSetEntityFragmentsValues(CreationContext->GetChunkCollection(), FragmentInstances);

	// Set unit index for entities
	FRTSFormationAgent FormationAgent;
	FormationAgent.UnitIndex = UnitIndex;
	
	TArray<FInstancedStruct> Fragments;
	Fragments.Add(FConstStructView::Make(FormationAgent));
	EntitySubsystem->BatchSetEntityFragmentsValues(CreationContext->GetChunkCollection(), Fragments);
}

int URTSFormationSubsystem::SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position)
{
	int UnitIndex = Units.Num();
	Units.AddDefaulted(1);
	Units[UnitIndex].UnitPosition = Position;
	
	SpawnEntitiesForUnit(UnitIndex, EntityConfig, Count);
	return UnitIndex;
}

void URTSFormationSubsystem::SetFormationPreset(int UnitIndex, UFormationPresets* FormationAsset)
{
	if (!ensure(FormationAsset && Units.IsValidIndex(UnitIndex))) { return; }
	
	FUnitInfo& Unit = Units[UnitIndex];
	Unit.FormationLength = FormationAsset->FormationLength;
	Unit.BufferDistance = FormationAsset->BufferDistance;
	Unit.Formation = FormationAsset->Formation;
	Unit.Rings = FormationAsset->Rings;
	Unit.bHollow = FormationAsset->bHollow;

	GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(FormationUpdated, Unit.Entities.Array());
}

void URTSFormationSubsystem::Tick(float DeltaTime)
{
	for(int i=0;i<Units.Num();++i)
	{
		FUnitInfo& Unit = Units[i];
		if (Unit.Formation != Circle)
		{
			Unit.InterpolatedAngle = LerpDegrees(Unit.InterpolatedAngle, Unit.Angle, DeltaTime);
		}
		
		Unit.InterpolatedDestination = FMath::VInterpConstantTo(Unit.InterpolatedDestination, Unit.UnitPosition, DeltaTime, 150.f);
	}
}

bool URTSFormationSubsystem::IsTickable() const
{
	return true;
}

TStatId URTSFormationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URTSFormationSubsystem, STATGROUP_Tickables);
}

void URTSFormationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	
}

float URTSFormationSubsystem::LerpDegrees(float start, float end, float amount)
{
	// Credit where credit is due! Definitely not complicated but rather out of convenience
	// https://stackoverflow.com/a/2708740
	float difference = abs(end - start);
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (end > start)
		{
			// We'll add it on to start...
			start += 360;
		}
		else
		{
			// Add it on to end.
			end += 360;
		}
	}

	// Interpolate it.
	float value = (start + ((end - start) * amount));

	// Wrap it..
	float rangeZero = 360;

	if (value >= 0 && value <= 360)
		return value;

	return fmod(value,rangeZero);
}
