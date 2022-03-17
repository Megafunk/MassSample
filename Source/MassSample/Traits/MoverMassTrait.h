// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "UObject/Object.h"
#include "MoverMassTrait.generated.h"

/**
 * 
 */

UCLASS(meta = (DisplayName = "Sample Mover Trait"))
class MASSSAMPLE_API UMoverMassTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	// FIXME: Not working as expected, revise processor file.
	UPROPERTY(EditAnywhere, Category = "Mass")
	FVector Velocity = {0,0,100.0f};

};
