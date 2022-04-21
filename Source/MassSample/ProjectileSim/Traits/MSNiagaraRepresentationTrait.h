// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"
#include "NiagaraSystem.h"
#include "MSNiagaraRepresentationTrait.generated.h"

/**
 * 
 */

UCLASS(meta = (DisplayName = "Niagara Representation"))
class MASSSAMPLE_API UMSNiagaraRepresentationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Config")
	UNiagaraSystem* SharedNiagaraSystem;

};

