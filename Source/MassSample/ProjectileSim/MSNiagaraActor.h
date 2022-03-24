// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraActor.h"
#include "GameFramework/Actor.h"
#include "MSNiagaraActor.generated.h"

UCLASS()
class MASSSAMPLE_API AMSNiagaraActor : public ANiagaraActor
{
	GENERATED_BODY()

public:
	//todo: do we really need ObjectInitializer because the parent class doesn't have another ctr? maybe?
	AMSNiagaraActor(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//these arrays are sent to our niagara component by the niagara representation processor
	//Why are they here instead of the fragment? dynamically sized data types are not allowed in mass due to the way archetypes are stored
	TArray<FVector> ParticlePositions;
	TArray<FVector> ParticleDirectionVectors;

};
