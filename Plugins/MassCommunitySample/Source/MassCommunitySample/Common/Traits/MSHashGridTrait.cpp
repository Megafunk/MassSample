// Fill out your copyright notice in the Description page of Project Settings.


#include "MSHashGridTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSOctreeFragments.h"

void UMSHashGridTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMSOctreeFragment>();
}
