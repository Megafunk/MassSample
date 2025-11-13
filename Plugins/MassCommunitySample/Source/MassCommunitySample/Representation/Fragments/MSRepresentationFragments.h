#pragma once

#include "MassEntityTypes.h"
#include "Representation/MSNiagaraActor.h"
#include "MSRepresentationFragments.generated.h"

/**	
 *	
**/
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AMSNiagaraActor> NiagaraManagerActor;
};
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FCustomNiagaraFloatsPairFragment : public FMassFragment
{
	GENERATED_BODY()
	float FirstFloat = 0.0f;
	float SecondFloat = 0.0f;
};


// This one is for responding to one-off events like a hit effect.
// I would argue it should be mapped based off of the physmat or something but oh well! 
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FSharedNiagaraSystemSpawnFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AMSNiagaraActor> NiagaraManagerActor;

	//This is used to make sure we insert to the right space in the niagara array after iterating a chunk and so on
	UPROPERTY()
	TArray<FVector> ParticlePositions;
	UPROPERTY(EditAnywhere)
	FName ParticlePositionsParameterName;

	
	UPROPERTY()
	TArray<FVector> ParticleDirectionVectors;
	
	UPROPERTY(EditAnywhere)
	FName ParticleDirectionVectorsParameterName;
	
	UPROPERTY(EditAnywhere)
	FName AuxParameterName;

};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FSampleISMPerInstanceSingleFloatFragment : public FMassFragment
{
	GENERATED_BODY()
	float Data = 0.0f;
};



