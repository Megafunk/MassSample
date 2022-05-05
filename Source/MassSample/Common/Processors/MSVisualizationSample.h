// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"

#include "MSVisualizationSample.generated.h"


/**
 * Example processors demonstrating how to use the representation module
 */
UCLASS()
class MASSSAMPLE_API UMSSampleVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:

	UMSSampleVisualizationProcessor()
	{
		bAutoRegisterWithProcessingPhases = true;
	};

};

UCLASS()
class MASSSAMPLE_API UMSSampleVisualizationLODProcessor : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()

public:

	UMSSampleVisualizationLODProcessor()
	{
		bAutoRegisterWithProcessingPhases = true;
	};

};