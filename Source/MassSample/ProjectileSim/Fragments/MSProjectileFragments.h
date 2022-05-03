#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GameplayEffect.h"
#include  "MSProjectileFragments.generated.h"
 
/**
* Fragments	
**/
USTRUCT()
struct MASSSAMPLE_API FDamageFragment : public FMassFragment
{
	GENERATED_BODY()
	float Damage;
};

// FIXME: Performance investigate making part of this shared? it's going to be very common besides the ignored actors etc?
USTRUCT(BlueprintType)
struct MASSSAMPLE_API FLineTraceFragment : public FMassFragment
{
	GENERATED_BODY()
	FCollisionQueryParams QueryParams = FCollisionQueryParams();

	
};

USTRUCT()
struct MASSSAMPLE_API FHitResultFragment : public FMassFragment
{
	FHitResultFragment() = default;

	explicit FHitResultFragment(const FHitResult& HitResult)
		: HitResult(HitResult)
	{
	}

	GENERATED_BODY()
	FHitResult HitResult;

};

// TODO: Move this elsewhere? It's not entirely projectile specific
USTRUCT()
struct MASSSAMPLE_API FLifeTimeFragment : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};

/**	Please keep in mind that we key FSharedNiagaraSystemFragment off of the pointer
*	to the niagara system select in the trait.
*	Don't use the regular struct CRC32 hash like you would for other shared fragments.
**/
USTRUCT()
struct MASSSAMPLE_API FSharedNiagaraSystemFragment : public FMassSharedFragment
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

/**
* Tags	
**/
USTRUCT()
struct MASSSAMPLE_API FStopMovementTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MASSSAMPLE_API FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MASSSAMPLE_API FFireHitEventTag : public FMassTag
{
	GENERATED_BODY()
};
