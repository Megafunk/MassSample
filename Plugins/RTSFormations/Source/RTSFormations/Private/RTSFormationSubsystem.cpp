// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
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

	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 5.f, 0, 25.f);
	
	// Calculate turn direction and angle for entities in unit
	float OldAngle = Units[UnitIndex].Angle * Units[UnitIndex].TurnDirection;
	Units[UnitIndex].TurnDirection = (NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal().Y > 0 ? 1.f : -1.f;
	Units[UnitIndex].Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(FVector::ForwardVector, (NewPosition-Units[UnitIndex].UnitPosition).GetSafeNormal())));
	Units[UnitIndex].Angle += 180.f; // Temporary fix to resolve unit facing the wrong direction
	
	// This might be stupid slow
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());

	struct SortNewPos
	{
		FVector Position;
		int Index;

		bool operator==(const SortNewPos& Other) const
		{
			return Other.Position == this->Position;
		}
	};
	
	TArray<SortNewPos> NewPositions;
	NewPositions.Reserve(Units[UnitIndex].Entities.Num());
	const FVector CenterOffset = FVector(0.f, (Units[UnitIndex].FormationLength/2) * Units[UnitIndex].BufferDistance, 0.f);
	
	for(int l=0;l<Units[UnitIndex].FormationLength;++l)
	{
		for(int w=0;w<FMath::RoundFromZero(Units[UnitIndex].Entities.Num()/Units[UnitIndex].FormationLength);++w)
		{
			SortNewPos EntityPosition;
			EntityPosition.Position = FVector(w,l,0.f);
			EntityPosition.Position *= Units[UnitIndex].BufferDistance;
			EntityPosition.Position -= CenterOffset;
			EntityPosition.Position = EntityPosition.Position.RotateAngleAxis(Units[UnitIndex].Angle, FVector(0.f,0.f,Units[UnitIndex].TurnDirection));
			EntityPosition.Position += NewPosition;
			EntityPosition.Index = NewPositions.Num();
			NewPositions.Emplace(EntityPosition);
		}
	}

	FVector UnitPosition = Units[UnitIndex].UnitPosition;
	NewPositions.Sort([&](const SortNewPos& A, const SortNewPos& B)
	{
		return FVector::Dist(A.Position, UnitPosition) > FVector::Dist(B.Position, UnitPosition);
	}); 

	Units[UnitIndex].Entities.Sort([&EntitySubsystem, &NewPositions, &NewPosition](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		const FVector& LocA = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(A).GetTransform().GetLocation();
		const FVector& LocB = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(B).GetTransform().GetLocation();
		return FVector::Dist(LocA, NewPosition) < FVector::Dist(LocB, NewPosition);
	});

	// We should not be doing this but for the convenience of the current logic in place, this is the fastest option i think
	for(int i=0;i<Units[UnitIndex].Entities.Num();i+=7)
	{
		TArray<SortNewPos> NewPositionsGroup;
		TArray<FMassEntityHandle> EntityGroup;
		EntityGroup.Reserve(7);
		NewPositionsGroup.Reserve(7);
		for(int j=0;j<7;++j)
		{
			int Index = j+(i/7*7);
			// Return early if there are no more entities
			if(!Units[UnitIndex].Entities.IsValidIndex(Index))
				break;
			
			EntityGroup.Emplace(Units[UnitIndex].Entities[Index]);
			NewPositionsGroup.Emplace(NewPositions[Index]);
		}
		for(const FMassEntityHandle& Entity : EntityGroup)
		{
			const FVector& EntityPos = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
			SortNewPos ClosestPos;
			float ClosestDist = -1;
			for(const SortNewPos& NewPos : NewPositionsGroup)
			{
				if(ClosestDist == -1 || FVector::Dist(NewPos.Position, EntityPos) < ClosestDist)
				{
					ClosestDist = FVector::Dist(NewPos.Position, EntityPos);
					ClosestPos = NewPos;
				}
			}
			EntitySubsystem->GetFragmentDataPtr<FRTSFormationAgent>(Entity)->Position = ClosestPos.Position;
			NewPositionsGroup.Remove(ClosestPos);
		}
	}
	
	Units[UnitIndex].UnitPosition = NewPosition;

	// Signal entities of a position update
	if (Units[UnitIndex].Entities.Num() > 0)
	{
		for(int i=0;i<Units[UnitIndex].Entities.Num();++i)
		{
			GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntity(FormationUpdated, Units[UnitIndex].Entities[i], i/7*0.25);
		}
	}
		//GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(FormationUpdated, Units[UnitIndex].Entities);
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
