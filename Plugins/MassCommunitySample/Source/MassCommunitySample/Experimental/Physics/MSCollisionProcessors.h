// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MassProcessor.h"

#include "MSCollisionProcessors.generated.h"
/**
 * Queries the Mass Sample octree
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSEntityCollisionQueryProcessors : public UMassProcessor
{
	GENERATED_BODY()

	UMSEntityCollisionQueryProcessors();
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;

	// A mass query that... queries an octree!
	FMassEntityQuery OctreeQueryQuery = FMassEntityQuery();

	UPROPERTY()
	 TObjectPtr<class UMSSubsystem> MSSubsystem;
};


