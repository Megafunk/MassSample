// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MSSubsystem.h"
#include "MSOctreeProcessors.generated.h"


UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSOctreeProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	
	UMSOctreeProcessor();
	
	FMassEntityQuery AddToHashGridQuery;
	FMassEntityQuery UpdateOctreeQuery;
	FMassEntityQuery RemoveFromGridEntityQuery;
	
	UPROPERTY()
	UMSSubsystem* MassSampleSystem;

	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSHashGridMemberInitializationProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	
	UMSHashGridMemberInitializationProcessor();
	
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	UMSSubsystem* MassSampleSystem;
	
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
