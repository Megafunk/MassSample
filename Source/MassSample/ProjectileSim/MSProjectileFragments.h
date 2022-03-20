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


/*		Please keep in mind that we key FSharedNiagaraSystemFragment off of the pointer
 *		to the niagara system select in the trait.
 *		Don't use the regular struct CRC32 hash like you would for other shared fragments.
*/
USTRUCT()
struct MASSSAMPLE_API FSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<class AMSNiagaraActor> NiagaraManagerActor;
	
	int32 IteratorOffset = 0;
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
