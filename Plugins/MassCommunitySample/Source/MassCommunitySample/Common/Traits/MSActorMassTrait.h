// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MSActorMassTrait.generated.h"

/**
 * 
 */

UCLASS(meta = (DisplayName = "Sample Actor Trait"))
class MASSCOMMUNITYSAMPLE_API UMSActorMassTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

};

