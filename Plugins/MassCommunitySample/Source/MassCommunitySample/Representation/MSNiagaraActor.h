// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NiagaraActor.h"
#include "MSNiagaraActor.generated.h"

UCLASS()
class MASSCOMMUNITYSAMPLE_API AMSNiagaraActor : public ANiagaraActor
{
	GENERATED_BODY()
public:
	//This is used to make sure we insert to the right space in the niagara array after iterating a chunk and so on
	int32 IteratorOffset = 0;
	
	TArray<FVector> ParticlePositions;

	TArray<FQuat4f> ParticleOrientations;
};
