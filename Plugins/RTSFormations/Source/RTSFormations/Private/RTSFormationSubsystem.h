// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Misc/MSBPFunctionLibrary.h"
#include "RTSFormationSubsystem.generated.h"

UENUM()
enum FormationType
{
	Rectangle,
	Circle
};

USTRUCT(BlueprintType)
struct FUnitInfo
{
	GENERATED_BODY()
	
public:
	// Entities in the unit
	UPROPERTY()
	TArray<FMassEntityHandle> Entities;

	// The current unit position
	UPROPERTY(BlueprintReadOnly)
	FVector UnitPosition;

	// The direction to turn the unit when rotating
	UPROPERTY()
	float TurnDirection = 1.f;

	// The entity length of the 'front' of the unit
	UPROPERTY()
	float FormationLength = 0;

	// The type of formation - WIP
	UPROPERTY()
	TEnumAsByte<FormationType> Formation = Rectangle;

	// The angle of the unit
	UPROPERTY()
	float Angle = 0;

	FUnitInfo() {};
};

class UMassAgentComponent;
struct FMassEntityHandle;
/**
 * Subsystem that handles the bulk of data shared among entities for the formation system. Enables simple unit creation and entity spawning
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Stores the num of units in the formation
	UPROPERTY(BlueprintReadOnly)
	TArray<FUnitInfo> Units;

	// Destroy a specified entity
	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);

	// Set the position of a unit
	UFUNCTION(BlueprintCallable)
	void SetUnitPosition(const FVector& NewPosition, int UnitIndex = 0);

	// Spawn entities for a unit
	UFUNCTION(BlueprintCallable)
	void SpawnEntitiesForUnit(int UnitIndex, const UMassEntityConfigAsset* EntityConfig, int Count);

	// Spawn a new unit
	UFUNCTION(BlueprintCallable)
	int SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position);
};
