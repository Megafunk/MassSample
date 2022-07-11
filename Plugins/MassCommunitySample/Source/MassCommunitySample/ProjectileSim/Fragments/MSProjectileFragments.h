#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GameplayEffect.h"
#include  "MSProjectileFragments.generated.h"
 
/**
* Fragments	
**/
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FDamageFragment : public FMassFragment
{
	GENERATED_BODY()
	float Damage;
};

// FIXME: Performance investigate making part of this shared? it's going to be very common besides the ignored actors etc?
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FLineTraceFragment : public FMassFragment
{
	GENERATED_BODY()
	FCollisionQueryParams QueryParams = FCollisionQueryParams();
};

// TODO: Move this elsewhere? It's not entirely projectile specific
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FHitResultFragment : public FMassFragment
{
	GENERATED_BODY()
	FHitResultFragment() = default;

	explicit FHitResultFragment(const FHitResult& HitResult)
		: HitResult(HitResult)
	{
	}

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FHitResult HitResult;

};

// TODO: Move this elsewhere? It's not entirely projectile specific
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FLifeTimeFragment : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};

/**
* Tags	
**/
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FStopMovementTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FFireHitEventTag : public FMassTag
{
	GENERATED_BODY()
};
