// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSMovementProcessor.generated.h"

// FIXME: This hello world is perfect as is, we just need to investigate the velocity fragment missbehav and document it.

/**
 * Example processor demonstrating how to move entities found in query
 * Processors are called "Systems" in most ECS libraries. 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:

	UMSMovementProcessor();

protected:

	//Note that we declare this ourselves! You can have many queries if need be.
	FMassEntityQuery MovementEntityQuery;

	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

};

