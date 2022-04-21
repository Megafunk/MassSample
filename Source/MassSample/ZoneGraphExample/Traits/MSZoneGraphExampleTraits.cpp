// Fill out your copyright notice in the Description page of Project Settings.


#include "MSZoneGraphExampleTraits.h"



#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
//#include "Fragments/MSFragments.h"
#include "ZoneGraphExample/Fragments/MSZoneGraphExampleFragments.h"

void UMSZoneGraphPathTestFromTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	// Fragments init //
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment_GetRef<FZoneGraphPathTestFromFragment>().FromTransform = FromTransform;
}

void UMSZoneGraphPathTestToTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	// Fragments init //
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment_GetRef<FZoneGraphPathTestToFragment>().ToTransform = ToTransform;
}



