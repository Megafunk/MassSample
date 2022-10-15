// Fill out your copyright notice in the Description page of Project Settings.


#include "MSDebugTagTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSFragments.h"

void UMSDebugTagTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMassSampleDebuggableTag>();
	BuildContext.AddFragment<FSampleColorFragment>();

}
