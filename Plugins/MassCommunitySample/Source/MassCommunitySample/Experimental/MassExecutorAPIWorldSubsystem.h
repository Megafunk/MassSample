// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MassProcessor.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassExecutorAPIWorldSubsystem.generated.h"

struct FMassEntityManager;

// Currently almost identical to UMassTestProcessorAutoExecuteQuery, this exists just to expose AutoExecuteQuery
UCLASS()
class UMassSampleNewAPIProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMassSampleNewAPIProcessor() {
		bAutoRegisterWithProcessingPhases = false; // We want these to be initialized manually
	}
	void SetAutoExecuteQuery(TSharedPtr<UE::Mass::FQueryExecutor> InAutoExecuteQuery)
	{
		AutoExecuteQuery = InAutoExecuteQuery;
	}
	FMassEntityQuery EntityQuery{ *this };
};
/**
 *  An example scratchpad of some newer APIs added by Epic (not me) in recent versions of Mass (post 5.6 for example)
 *  this really just exists so I can hook into beginplay 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMassExecutorAPIWorldSubsystem : public UWorldSubsystem {
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	TSharedPtr<FMassEntityManager> ĘntityManager;
	
	UPROPERTY()
	TArray<TObjectPtr<UMassSampleNewAPIProcessor>> Processors;
};
