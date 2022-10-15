#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include  "MSRepresentationFragments.generated.h"

/**	Please keep in mind that we key NiagaraSystemFragments off of the pointer
*	to the niagara system selected in the trait.
*	Don't use the regular struct CRC32 hash like you would for other shared fragments.
**/
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<class AMSNiagaraActor> NiagaraManagerActor;

	//This is used to make sure we insert to the right space in the niagara array after iterating a chunk and so on
	int32 IteratorOffset = 0;


	UPROPERTY(EditAnywhere)
	FName ParticlePositionsName = "MassParticlePositions";
	UPROPERTY()
	TArray<FVector> ParticlePositions;
	
	UPROPERTY(EditAnywhere)
	FName ParticleDirectionsParameterName = "MassParticleDirectionVectors";

	UPROPERTY()
	TArray<FVector> ParticleDirections;

};

// This one is for responding to one-off events like a hit effect.
// I would argue it should be mapped based off of the physmat or something but oh well! 
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FSharedNiagaraSystemSpawnFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<class AMSNiagaraActor> NiagaraManagerActor;

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