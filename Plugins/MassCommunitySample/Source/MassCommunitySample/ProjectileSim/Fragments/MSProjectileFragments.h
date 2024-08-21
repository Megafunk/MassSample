#pragma once
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "MassEntityTypes.h"
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

	TArray<AActor*,TInlineAllocator<2>> IgnoredActors;
};



// This would arguably make more sense as a shared thing but it's only a byte...
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSCollisionChannelFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> Channel = ECC_Camera;
};

/**
* Tags	
**/

// A lot of these could use some data but for now I will just use them as flags

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSLineTraceTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSOctreeQueryTag : public FMassTag
{
	GENERATED_BODY()
};
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSProjectileFireHitEventTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSProjectileStopOnHitTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSProjectileRicochetTag : public FMassTag
{
	GENERATED_BODY()
};
USTRUCT(BlueprintType)
struct MASSCOMMUNITYSAMPLE_API FMSProjectileTag : public FMassTag
{
	GENERATED_BODY()
};


