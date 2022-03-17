// Fill out your copyright notice in the Description page of Project Settings.


#include "MoverMassTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "Fragments/MassSampleFragments.h"

void UMoverMassTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();

	BuildContext.AddFragment_GetRef<FMassVelocityFragment>().Value = Velocity;
	
	BuildContext.AddTag<FMoverTag>();
	
}
