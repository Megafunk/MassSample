// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSNavMeshProcessors.generated.h"




/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNavMeshProcessors : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSNavMeshProcessors();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
public:
};
