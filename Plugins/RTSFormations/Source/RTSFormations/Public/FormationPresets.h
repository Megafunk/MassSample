// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FormationPresets.generated.h"

UENUM()
enum FormationType
{
	Rectangle,
	Circle
};

/**
 * 
 */
UCLASS(Blueprintable)
class RTSFORMATIONS_API UFormationPresets : public UDataAsset
{
	GENERATED_BODY()
public:
	
	// The type of formation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<FormationType> Formation = Rectangle;
	
	// The formation length of the unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FormationLength = 8;

	// Distance between units
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BufferDistance = 100.f;

	// Amount of rings for the circle formation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Rings = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHollow = false;
};
