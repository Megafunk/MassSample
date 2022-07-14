// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "MassAgentComponent.h"
#include "MassEntitySubsystem.h"
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

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, int UnitIndex)
{
	if (!ensure(Units.IsValidIndex(UnitIndex))) { return; }

	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 10.f, 0, 25.f);

	// Calculate turn direction and angle for entities in unit
	Units[UnitIndex].TurnDirection = (NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal().Y > 0 ? 1.f : -1.f;
	Units[UnitIndex].Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(FVector::ForwardVector, (NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal())));
	Units[UnitIndex].Angle += 180.f; // Temporary fix to resolve unit facing the wrong direction
	
	Units[UnitIndex].UnitPosition = NewPosition;

	// Signal entities of a position update
	if (Units[UnitIndex].Entities.Num() > 0)
		GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(FormationUpdated, Units[UnitIndex].Entities);
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
