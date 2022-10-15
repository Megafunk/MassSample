#pragma once
#include "CoreMinimal.h"
#include  "MSRepresentationTypes.generated.h"


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FNiagaraMassHelper
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<class AMSNiagaraActor> NiagaraManagerActor;

	//This is used to make sure we insert to the right space in the niagara array after iterating a chunk and so on
	int32 IteratorOffset = 0;
	
	UPROPERTY()
	TArray<FVector> ParticlePositions;
	

	UPROPERTY()
	TArray<FVector> ParticleDirectionVectors;

};

