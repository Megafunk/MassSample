// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectileSimTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MSProjectileFragments.h"
#include "NiagaraSystem.h"

void UMSProjectileSimTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FLineTraceFragment>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassVelocityFragment>();
	BuildContext.AddTag<FProjectileTag>();
}
