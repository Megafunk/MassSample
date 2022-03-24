// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MSProjectileFragments.h"
#include "NiagaraSystem.h"
#include "MSProjectileSimTrait.generated.h"

/**
 * 
 */

UCLASS(meta = (DisplayName = "Pojectile Simulation"))
class MASSSAMPLE_API UMSProjectileSimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
	

};

