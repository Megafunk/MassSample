// Fill out your copyright notice in the Description page of Project Settings.


#include "MSAssortedTags.h"

#include "MassEntityTemplateRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSAssortedTags)

void UDEPRECATED_UMSAssortedTags::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	for (const FInstancedStruct& Tag : Tags)
	{
		if(!Tag.IsValid())
		{
			continue;
		}
		const UScriptStruct* StructType = Tag.GetScriptStruct();
		if (StructType->IsChildOf(FMassTag::StaticStruct()))
		{
			BuildContext.AddTag(*StructType);
		}
	}
}
