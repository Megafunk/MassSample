#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GameplayEffect.h"
#include  "MSProjectileFragments.generated.h"
 

/*
 *   Fragments	
 */



USTRUCT()
struct MASSSAMPLE_API FMassFragment_Projectile : public FMassFragment
{
	GENERATED_BODY()
	float Damage;
};

//the idea is to use this for representation and linetraces mainly
USTRUCT()
struct MASSSAMPLE_API FMassFragment_PreviousPos : public FMassFragment
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	FVector Value;
};

USTRUCT()
struct MASSSAMPLE_API FMassFragment_LineTrace : public FMassFragment
{
	GENERATED_BODY()
	//this is honestly a fairly large  struct, do I need the full hitresult? leave it for now I suppose.
	FHitResult HitResult; 
};

//TODO: move this elsewhere?
USTRUCT()
struct MASSSAMPLE_API FMFragment_LifeTime : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};


USTRUCT()
struct MASSSAMPLE_API FSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()
	TWeakObjectPtr<class UNiagaraSystem> NiagaraSystem; 
};



/*
 *   Tags	
 */
USTRUCT()
struct MASSSAMPLE_API FTag_StopMovement : public FMassTag
{
	GENERATED_BODY()
};
USTRUCT()
struct MASSSAMPLE_API FMassFragment_CollisionHit : public FMassTag
{
	GENERATED_BODY()
};
