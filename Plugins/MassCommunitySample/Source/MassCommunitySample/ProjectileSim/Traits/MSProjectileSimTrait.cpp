// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"

void UMSProjectileSimTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FLineTraceFragment>();
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();
	BuildContext.AddTag<FProjectileTag>();

	if (bFiresHitEventToActors)
	{
		BuildContext.AddTag<FFireHitEventTag>();
	}
}
