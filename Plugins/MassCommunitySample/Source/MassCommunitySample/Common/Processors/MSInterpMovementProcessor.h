// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
