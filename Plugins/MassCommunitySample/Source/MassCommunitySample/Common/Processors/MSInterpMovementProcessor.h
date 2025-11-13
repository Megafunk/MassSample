// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MassProcessor.h"
#include "MSInterpMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSInterpMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSInterpMovementProcessor();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
