// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MSZoneGraphPathTestTraits.generated.h"





/**
 * Set the transform to the place that the path will be starting at.
 */
UCLASS(meta=(DisplayName="Path From Trait"))
class MASSSAMPLE_API UMSZoneGraphPathTestFromTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()


	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;


	/** Set to location from */
	UPROPERTY(Category = "Path2Mass", EditAnywhere)
	FTransform FromTransform = FTransform(FVector::ZeroVector);
	
};

/*
 * Set the transform to the place that the path will be ending at.
 */
UCLASS(meta=(DisplayName="Path To Trait"))
class MASSSAMPLE_API UMSZoneGraphPathTestToTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()


	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;


	/** Set to location to */
	UPROPERTY(Category = "Path2Mass", EditAnywhere)
	FTransform ToTransform = FTransform(FVector::ZeroVector);
	
};
