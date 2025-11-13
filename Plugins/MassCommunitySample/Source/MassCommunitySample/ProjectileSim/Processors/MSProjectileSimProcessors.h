// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define MASSSAMPLE_PARALLEL_OCTREE_QUERY 1


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
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;
	

	FMassEntityQuery LineTraceFromPreviousPosition;

};


