// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MSProjectileSimTrait.generated.h"

/**
 * A projectile that line traces from where it was last frame (using velocity) to find hits. Other processors move it!
 */

UCLASS(meta = (DisplayName = "Pojectile Simulation"))
class MASSCOMMUNITYSAMPLE_API UMSProjectileSimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	// Whether to try to call the IMassProjectileHitInterface on actors we hit. This just adds a tag to the template conditionally 
	UPROPERTY(EditAnywhere)
	bool bFiresHitEventToActors = true;
};
