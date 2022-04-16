// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphPathing/MSZoneGraphPathTestTraits.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Fragments/MSFragments.h"

void UMSZoneGraphPathTestFromTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	//Super::BuildTemplate(BuildContext, World);

	// Here for reference //
	//BuildContext.AddTag<xxxxxxxxxxxx>();
	//BuildContext.AddFragment<xxxxxxxxxxxx>();
	//BuildContext.AddFragment_GetRef<FCrowdFollowTargetFragment>().ZoneGraphTag = DisturbanceAnnotationTagFilter;

	// Fragments init //
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment_GetRef<FZoneGraphPathTestFromFragment>().FromTransform = FromTransform;
}

void UMSZoneGraphPathTestToTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	//Super::BuildTemplate(BuildContext, World);

	// Here for reference //
	//BuildContext.AddTag<xxxxxxxxxxxx>();
	//BuildContext.AddFragment<xxxxxxxxxxxx>();
	//BuildContext.AddFragment_GetRef<FCrowdFollowTargetFragment>().ZoneGraphTag = DisturbanceAnnotationTagFilter;

	// Fragments init //
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment_GetRef<FZoneGraphPathTestToFragment>().ToTransform = ToTransform;
}
