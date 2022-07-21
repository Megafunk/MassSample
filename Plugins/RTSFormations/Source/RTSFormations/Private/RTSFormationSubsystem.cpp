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
	
	TSet<FSortNewPos> NewPositions;
	NewPositions.Reserve(Unit.Entities.Num());

	//Idea
	// Rotate and set position to closest entity to destination (done)
	// Then, from the farthest unit, calculate the closest destination
	const FVector CenterOffset = FVector((Unit.Entities.Num()/Unit.FormationLength/2) * Unit.BufferDistance, (Unit.FormationLength/2) * Unit.BufferDistance, 0.f);

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("CalculateNewPositions"))
		for(int l=0;l<Unit.FormationLength;++l)
		{
			for(int w=0;w<FMath::RoundFromZero(Unit.Entities.Num()/float(Unit.FormationLength));++w)
			{
				FSortNewPos EntityPosition;
				EntityPosition.Position = FVector(w,l,0.f);
				EntityPosition.Position *= Unit.BufferDistance;
				EntityPosition.Position -= CenterOffset;
				EntityPosition.Position = EntityPosition.Position.RotateAngleAxis(Unit.Angle, FVector(0.f,0.f,Unit.TurnDirection));
				EntityPosition.Position += NewPosition;
				EntityPosition.Index = NewPositions.Num();
				NewPositions.Emplace(EntityPosition);
			}
		}
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("SortPositions"))
		Unit.Entities.Sort([&EntitySubsystem, &NewPosition](const FMassEntityHandle& A, const FMassEntityHandle& B)
		{
			const FVector& LocA = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(A).GetTransform().GetLocation();
			const FVector& LocB = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(B).GetTransform().GetLocation();
			return FVector::Dist(LocA, NewPosition) < FVector::Dist(LocB, NewPosition);
		});
	}

	// We should not be doing this but for the convenience of the current logic in place, this is the fastest option i think
	// This overcomplicated logic is to make sure each 'group' of entities chooses the closest destination possible.
	// @todo performance and transfer to processor?
	TSet<int> IndexesTaken;
	IndexesTaken.Reserve(Unit.Entities.Num());

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("SetEntityIndex"))
		for(int i=0;i<Unit.Entities.Num();++i)
		{
			const FVector& Location = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(Unit.Entities[i])->GetTransform().GetLocation();
			float ClosestDistance = -1;
			FSortNewPos ClosestPos = FSortNewPos();
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("DetermineClosestPosition"))
				for(const FSortNewPos& NewPos : NewPositions)
				{
					const float Dist = FVector::Dist(Location, NewPos.Position);
					if (ClosestDistance == -1 || Dist < ClosestDistance)
					{
						ClosestDistance = Dist;
						ClosestPos = NewPos;
					}
				}
			} 
		
			// Basically scoot up entities if there is space in the front
			int Index = ClosestPos.Index;
			while(!IndexesTaken.Contains(Index-Unit.FormationLength) && Index-Unit.FormationLength >= 0)
			{
				Index -= Unit.FormationLength;
			}
			
			IndexesTaken.Emplace(Index);
			EntitySubsystem->GetFragmentDataPtr<FRTSFormationAgent>(Unit.Entities[i])->EntityIndex = Index;
			NewPositions.Remove(ClosestPos);
		}
	}
}

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, int UnitIndex)
{
	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 5.f, 0, 25.f);
	
	// Calculate turn direction and angle for entities in unit
	Units[UnitIndex].TurnDirection = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal().Y > 0 ? 1.f : -1.f;
	Units[UnitIndex].Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct((NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal(),FVector::ForwardVector)));
	Units[UnitIndex].ForwardVector = (NewPosition-Units[UnitIndex].InterpolatedDestination).GetSafeNormal();
	Units[UnitIndex].Angle += 180.f; // Temporary fix to resolve unit facing the wrong direction
	
	Units[UnitIndex].UnitPosition = NewPosition;

	// Instantly set the angle since we are not keeping complete formation right now
	Units[UnitIndex].InterpolatedAngle = Units[UnitIndex].Angle;
	UpdateUnitPosition(NewPosition, UnitIndex);

	CurrentIndex = 0;

	// Sort entities from front-to-back
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	Units[UnitIndex].Entities.Sort([&](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		const int LocA = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(A).EntityIndex;
		const int LocB = EntitySubsystem->GetFragmentDataChecked<FRTSFormationAgent>(B).EntityIndex;
		return LocA < LocB;
	});

	// Signal entities to begin moving
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]()
	{
		// Signal entities of a position update
		if (Units[0].Entities.Num() > 0)
		{
			CurrentIndex = FMath::Clamp(CurrentIndex+1, 0, Units[0].Entities.Num()-1);
			GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntity(FormationUpdated, Units[0].Entities[CurrentIndex]);
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
