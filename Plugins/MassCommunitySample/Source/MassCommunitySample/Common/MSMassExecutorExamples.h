// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassQueryExecutor.h"
#include "MassSignalProcessorBase.h"
#include "MSMassExecutorExamples.generated.h"


// An example of a new Mass FQueryExecutor
UCLASS()
class UMSSignalProcessorExecutorExample : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	UMSSignalProcessorExecutorExample();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
	virtual void OnSignalReceived(FName SignalName, TConstArrayView<FMassEntityHandle> Entities) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
};