// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define MASSSAMPLE_PARALLEL_OCTREE_QUERY 1

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSProjectileSimProcessors.generated.h"
/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSProjectileSimProcessors : public UMassProcessor
{
	GENERATED_BODY()

	UMSProjectileSimProcessors();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void Initialize(UObject& Owner) override;
	

	FMassEntityQuery LineTraceFromPreviousPosition;

};


