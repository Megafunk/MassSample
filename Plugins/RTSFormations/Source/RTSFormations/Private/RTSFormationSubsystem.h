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
	TSet<FMassEntityHandle> Entities;

	// The current unit position
	UPROPERTY(BlueprintReadOnly)
	FVector UnitPosition;

	// The direction to turn the unit when rotating
	UPROPERTY(BlueprintReadWrite)
	float TurnDirection = 1.f;

	// The entity length of the 'front' of the unit
	UPROPERTY(BlueprintReadWrite)
	int FormationLength = 8;

	UPROPERTY(BlueprintReadWrite)
	float BufferDistance = 100.f;

	// The type of formation - WIP
	UPROPERTY()
	TEnumAsByte<FormationType> Formation = Rectangle;

	// The angle of the unit
	UPROPERTY()
	float Angle = 0;

	UPROPERTY()
	bool bReverseUnit = false;

	UPROPERTY()
	FVector ForwardVector;

	// Interpolated movement
	UPROPERTY()
	FVector InterpolatedDestination;

	UPROPERTY()
	float InterpolatedAngle = 0.f;

	UPROPERTY()
	float InterpolationSpeed = 5.f;

	FUnitInfo() {};
};

class UMassAgentComponent;
struct FMassEntityHandle;
/**
 * Subsystem that handles the bulk of data shared among entities for the formation system. Enables simple unit creation and entity spawning
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Stores the num of units in the formation
	UPROPERTY(BlueprintReadWrite)
	TArray<FUnitInfo> Units;

	// Destroy a specified entity
	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);

	// Set the position of a unit
	UFUNCTION()
	void UpdateUnitPosition(const FVector& NewPosition, int UnitIndex = 0);

	UFUNCTION(BlueprintCallable)
	void SetUnitPosition(const FVector& NewPosition, int UnitIndex = 0);
	
	// Spawn entities for a unit
	UFUNCTION(BlueprintCallable)
	void SpawnEntitiesForUnit(int UnitIndex, const UMassEntityConfigAsset* EntityConfig, int Count);

	// Spawn a new unit
	UFUNCTION(BlueprintCallable)
	int SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position);

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UPROPERTY()
	FTimerHandle MoveHandle;
	
	int CurrentIndex = 0;
};
