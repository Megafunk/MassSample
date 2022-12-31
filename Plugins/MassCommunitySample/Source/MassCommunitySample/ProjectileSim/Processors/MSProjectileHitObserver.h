// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "UObject/Object.h"
#include "MSProjectileHitObserver.generated.h"

/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSProjectileHitObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
public:
	UMSProjectileHitObserver();
	
protected:
	
	virtual void ConfigureQueries() override;
	
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery ResolveHitsQuery;
	FMassEntityQuery RicochetHitsQuery;

	FMassEntityQuery CollisionHitEventQuery;

};
