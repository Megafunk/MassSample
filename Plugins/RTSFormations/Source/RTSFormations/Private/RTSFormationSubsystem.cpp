// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationProcessors.h"
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

	FUnitInfo& Unit = Units[UnitIndex];
	
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	
	TMap<int, FVector> NewPositions;
	NewPositions.Reserve(Unit.Entities.Num());

	//Idea
	// Rotate and set position to closest entity to destination (done)
	// Then, from the farthest unit, calculate the closest destination
	const FVector CenterOffset = FVector((Unit.Entities.Num()/Unit.FormationLength/2) * Unit.BufferDistance, (Unit.FormationLength/2) * Unit.BufferDistance, 0.f);

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("CalculateNewPositions"))
		for(int i=0;i<Unit.Entities.Num();++i)
		{
			const int w = i / Unit.FormationLength;
			const int l = i % Unit.FormationLength;
			
			FVector Position = FVector(w,l,0.f);
			Position *= Unit.BufferDistance;
			Position -= CenterOffset;
			Position = Position.RotateAngleAxis(Unit.Angle, FVector(0.f,0.f,Unit.TurnDirection));
			Position += NewPosition;
			NewPositions.Emplace(i, Position);
		}
	}
	FVector FrontCenter = FVector((Unit.Entities.Num()/Unit.FormationLength/2) * Unit.BufferDistance, 0.f, 0.f).RotateAngleAxis(Unit.Angle, FVector(0.f,0.f,Unit.TurnDirection));
	FVector FrontMidPosition = NewPosition - FrontCenter;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("SortPositions"))
		Unit.Entities.Sort([&EntitySubsystem, &FrontMidPosition](const FMassEntityHandle& A, const FMassEntityHandle& B)
		{
			// Find if theres a way to move this logic to a processor, most of the cost is coming from retrieving the location
			const FVector& LocA = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(A).GetTransform().GetLocation();
			const FVector& LocB = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(B).GetTransform().GetLocation();
			return FVector::DistSquared2D(LocA, FrontMidPosition) < FVector::DistSquared2D(LocB, FrontMidPosition);
		});
	}
	
	NewPositions.ValueSort([&FrontMidPosition](const FVector& A, const FVector& B)
	{
		return FVector::DistSquared2D(A, FrontMidPosition) < FVector::DistSquared2D(B, FrontMidPosition);
	});
	DrawDebugPoint(GetWorld(), FrontMidPosition, 20.f, FColor::Yellow, false, 10.f);

	// We should not be doing this but for the convenience of the current logic in place, this is the fastest option i think
	// @todo performance and transfer to processor?
	TSet<int> IndexesTaken;
	IndexesTaken.Reserve(Unit.Entities.Num());

	int i=0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("SetEntityIndex"))
		for(const FMassEntityHandle& Entity : Unit.Entities)
		{
			// Get first index since it is sorted
			TPair<int, FVector> ClosestPos;
			for(const TPair<int, FVector>& NewPos : NewPositions)
			{
				ClosestPos = NewPos;
				break;
			}

			// Basically scoot up entities if there is space in the front
			int& Index = ClosestPos.Key;
			
			IndexesTaken.Emplace(Index);
			EntitySubsystem->GetFragmentDataPtr<FRTSFormationAgent>(Entity)->EntityIndex = Index;
			
			//DrawDebugString(GetWorld(), *NewPositions.Find(Index), FString::Printf(TEXT("%d"), Index), NULL, FColor::Black, 10.f);
			NewPositions.Remove(Index);
			i++;
		}
	}
}

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, int UnitIndex)
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	
	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 5.f, 0, 25.f);

	FUnitInfo& Unit = Units[UnitIndex];
	// Calculate turn direction and angle for entities in unit
	Unit.TurnDirection = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal().Y > 0 ? 1.f : -1.f;
	Unit.Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal(),FVector::ForwardVector)));
	Unit.ForwardVector = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal();
	Unit.Angle += 180.f; // Temporary fix to resolve unit facing the wrong direction
	
	Unit.UnitPosition = NewPosition;

	// Instantly set the angle since we are not keeping complete formation right now
	Unit.InterpolatedAngle = Units[UnitIndex].Angle;
	UpdateUnitPosition(Unit.InterpolatedDestination, UnitIndex);

	CurrentIndex = 0;

	// Signal entities to begin moving
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]()
	{
		// Signal entities of a position update
		if (Units[0].Entities.Num() > 0)
		{
			CurrentIndex = FMath::Clamp(CurrentIndex+1, 0, Units[0].Entities.Num()-1);
			GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntity(FormationUpdated, Units[0].Entities.Array()[CurrentIndex]);
		}
	});
	GetWorld()->GetTimerManager().ClearTimer(MoveHandle);
	GetWorld()->GetTimerManager().SetTimer(MoveHandle, TimerDelegate, 0.05, true);
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

void URTSFormationSubsystem::Tick(float DeltaTime)
{
	for(int i=0;i<Units.Num();++i)
	{
		FUnitInfo& Unit = Units[i];
		Unit.InterpolatedAngle = FMath::FInterpConstantTo(Unit.InterpolatedAngle, Unit.Angle, DeltaTime, 50.f);
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
