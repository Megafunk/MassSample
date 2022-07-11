#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include  "MSProjectileFragments.generated.h"

/**	Please keep in mind that we key FSharedNiagaraSystemFragment off of the pointer
*	to the niagara system select in the trait.
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

	UPROPERTY()
	TArray<FVector> ParticlePositions;

	UPROPERTY()
	TArray<FVector> ParticleDirectionVectors;

};

USTRUCT()
struct FSampleISMPerInstanceSingleFloatFragment : public FMassFragment
{
	GENERATED_BODY()
	float Data = 0;
};