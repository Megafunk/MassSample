// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMoverMassTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "Common/Fragments/MSFragments.h"

void UMSMoverMassTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddTag<FMoverTag>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment_GetRef<FMassForceFragment>().Value = Force;
}
