// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassMovementFragments.h"
#include "MassProcessor.h"
#include "MSProjectileSimProcessors.generated.h"
/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSProjectileSimProcessors : public UMassProcessor
{
	GENERATED_BODY()

	UMSProjectileSimProcessors();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	
	virtual void Initialize(UObject& Owner) override;
	
	FMassEntityQuery LineTraceFromPreviousPosition;
};


