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

	UPROPERTY(BlueprintReadOnly)
	FVector UnitPosition;

	UPROPERTY()
	FVector ForwardDirection;

	UPROPERTY()
	float FormationLength = 0;

	UPROPERTY()
	TEnumAsByte<FormationType> Formation = Rectangle;

	UPROPERTY()
	float Angle = 0;

	FUnitInfo() {};

	FUnitInfo(const TArray<FMassEntityHandle>& Entities)
	{
		this->Entities.Append(Entities);
	}
};

class UMassAgentComponent;
struct FMassEntityHandle;
/**
 * 
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Stores the num of units in the formation
	UPROPERTY(BlueprintReadOnly)
	TArray<FUnitInfo> Units;

	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);

	UFUNCTION(BlueprintCallable)
	void SetUnitPosition(const FVector& NewPosition, int UnitIndex = 0);

	UFUNCTION(BlueprintCallable)
	void SpawnEntitiesForUnit(int UnitIndex, const UMassEntityConfigAsset* EntityConfig, int Count);

	UFUNCTION(BlueprintCallable)
	int SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count);
};
