// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "CollisionProcessors.generated.h"
/**
 * Queries the Mass Sample octree 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSEntityCollisionQueryProcessors : public UMassProcessor
{
	GENERATED_BODY()

	UMSEntityCollisionQueryProcessors();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void Initialize(UObject& Owner) override;

	// Yeah... I know
	FMassEntityQuery OctreeQueryQuery;

	UPROPERTY()
	class UMSSubsystem* MSSubsystem;;
};


