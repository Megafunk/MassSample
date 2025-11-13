// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "InstancedStruct.h"
#include "MassEntityTraitBase.h"
#include "StructUtils/InstancedStruct.h"
#include "MSAssortedTags.generated.h"

/**
 * Not working due to due to BuildContext.AddTag checking for fragments and not tags
 */
UCLASS(meta=(DisplayName="Assorted Tags"), Deprecated)
class MASSCOMMUNITYSAMPLE_API UDEPRECATED_UMSAssortedTags : public UMassEntityTraitBase
{
	GENERATED_BODY()
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/MassEntity.MassTag", ExcludeBaseStruct))
	TArray<FInstancedStruct> Tags;
};
