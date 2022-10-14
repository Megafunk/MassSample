// Fill out your copyright notice in the Description page of Project Settings.


#include "MSAssortedTags.h"

#include "MassEntityTemplateRegistry.h"

void UMSAssortedTags::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	for (auto& Tag : Tags)
	{
		const UScriptStruct* TagStruct = Tag.GetScriptStruct();

		if (TagStruct)
		{
			if(TagStruct->IsChildOf(FMassTag::StaticStruct()))
			{
				BuildContext.AddTag(*TagStruct);
			}
		}


	}
}
