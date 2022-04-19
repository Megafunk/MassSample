// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSEntityActorExampleProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSEntityActorExampleProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UMSEntityActorExampleProcessor();

	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	
};
