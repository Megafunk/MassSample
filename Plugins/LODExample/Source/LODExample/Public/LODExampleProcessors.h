// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassLODCollectorProcessor.h"
#include "LODExampleProcessors.generated.h"

//MassSample already includes MS

/**
 * Simple class to enable LODCollection for entities
 * @see UMassCrowdLODCollectorProcessor
 */
UCLASS()
class LODEXAMPLE_API ULODCollectorExampleProcessor : public UMassLODCollectorProcessor
{
	GENERATED_BODY()

	ULODCollectorExampleProcessor();
};

UCLASS()
class LODEXAMPLE_API ULODExampleProcessor : public UMassProcessor
{
	GENERATED_BODY()

	ULODExampleProcessor();

	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQueryBase;
	FMassEntityQuery EntityQuery_High;
	FMassEntityQuery EntityQuery_Medium;
	FMassEntityQuery EntityQuery_Low;
};
