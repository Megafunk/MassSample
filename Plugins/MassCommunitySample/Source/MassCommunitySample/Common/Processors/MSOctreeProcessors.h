// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MSSubsystem.h"
#include "MSOctreeProcessors.generated.h"


// Updates the sample's TOctree2 node positions every frame
// I would say this would better serve as a way of figuring out if we are near something rather than a straight up physics system
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSOctreeProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	
	UMSOctreeProcessor();
	
	FMassEntityQuery AddToHashGridQuery;
	FMassEntityQuery UpdateOctreeElementsQuery;
	FMassEntityQuery RemoveFromGridEntityQuery;
	
	UPROPERTY()
	TObjectPtr<UMSSubsystem> MassSampleSystem;

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void BeginDestroy() override;
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSHashGridMemberInitializationProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	
	UMSHashGridMemberInitializationProcessor();
	
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	TObjectPtr<UMSSubsystem> MassSampleSystem = nullptr;
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};


UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSOctreeMemberCleanupProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	
	UMSOctreeMemberCleanupProcessor();
	
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	TObjectPtr<UMSSubsystem> MassSampleSystem;
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
