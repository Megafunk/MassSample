// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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


UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSProjectileOctreeQueryProcessors : public UMassProcessor
{
	GENERATED_BODY()

	UMSProjectileOctreeQueryProcessors();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void Initialize(UObject& Owner) override;
	
	FMassEntityQuery ProjectileOctreeQuery;

	UPROPERTY()
	class UMSSubsystem* MSSubsystem;;
};


