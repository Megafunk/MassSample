#pragma once
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "MassEntityTypes.h"
#include "GameplayEffect.h"
#include  "MSProjectileFragments.generated.h"
 
/**
* Fragments	
**/
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FDamageFragment : public FMassFragment
{
	GENERATED_BODY()
	float Damage;
};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSCollisionIgnoredActorsFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<AActor*> IgnoredActors;
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
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FLifeTimeFragment : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};

/**
* Tags	
**/
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FStopMovementTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};

// Might add data to this later, it's pretty generic here?
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSLineTraceTag : public FMassTag
{
	GENERATED_BODY()
};
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FFireHitEventTag : public FMassTag
{
	GENERATED_BODY()
};
