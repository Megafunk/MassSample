// Fill out your copyright notice in the Description page of Project Settings.


#include "MSAssortedTags.h"

#include "MassEntityTemplateRegistry.h"

void UMSAssortedTags::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	for (auto& Tag : Tags)
	{
		const UScriptStruct* TagStruct = Tag.GetScriptStruct();

		if(TagStruct->IsChildOf(FMassTag::StaticStruct()))
		{
			BuildContext.AddTag(*TagStruct);
		}
	}
}
