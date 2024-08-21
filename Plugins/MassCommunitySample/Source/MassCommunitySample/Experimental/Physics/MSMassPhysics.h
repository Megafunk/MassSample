// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSMassPhysics.generated.h"
/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSChaosMassTranslationProcessorsProcessors : public UMassProcessor
{
	GENERATED_BODY()
	UMSChaosMassTranslationProcessorsProcessors();

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery ChaosSimToMass;

	FMassEntityQuery UpdateChaosKinematicTargets;
	FMassEntityQuery MassTransformsToChaosBodies;


};
