#pragma once

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
	
	// It IS largely fine to store a heap-allocated array in an archetype ECS as copying a pointer doesn't invalidate what it points to
	// Here I am going to be fancy and make this inline allocate 2 elements to avoid allocations as it's seamless to use
	TArray<TWeakObjectPtr<AActor>, TInlineAllocator<2>> IgnoredActors;
};

template<>
struct TMassFragmentTraits<FMSCollisionIgnoredActorsFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
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


