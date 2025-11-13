// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MassObserverProcessor.h"
#include "MSObserverSamples.generated.h"

/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSObserverOnAdd : public UMassObserverProcessor
{
	GENERATED_BODY()

public:

	UMSObserverOnAdd();
	
protected:
	
	FMassEntityQuery EntityQuery;

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
