// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MassEntityTraitBase.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "MSProjectileSimTrait.generated.h"

/**
 * A projectile that line traces from where it was last frame (using velocity) to find hits. Other processors move it!
 * This is just a simple example, a more complex project would probably need a more specific linetrace.
 */

UCLASS(meta = (DisplayName = "Pojectile Simulation"))
class MASSCOMMUNITYSAMPLE_API UMSProjectileSimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	// Whether to try to call the IMassProjectileHitInterface on actors we hit. This just adds a tag to the template conditionally 
	UPROPERTY(EditAnywhere)
	bool bFiresHitEventToActors = true;

	UPROPERTY(EditAnywhere)
	bool bRicochet = false;
	
	UPROPERTY(EditAnywhere)
	bool bHasGravity = true;

	UPROPERTY(EditAnywhere)
	bool bQueriesOctree = false;

	UPROPERTY(EditAnywhere)
	bool bRotationFollowsVelocity = true;

	UPROPERTY(EditAnywhere,meta=(ShowOnlyInnerProperties))
	FMSCollisionChannelFragment CollisionChannelFragment;
	
};
