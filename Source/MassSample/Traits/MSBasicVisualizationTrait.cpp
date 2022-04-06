// Fill out your copyright notice in the Description page of Project Settings.

#include "MSBasicVisualizationTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassActorSubsystem.h"

void UMSBasicVisualizationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	Super::BuildTemplate(BuildContext, World);
	
	BuildContext.AddFragment<FMassActorFragment>();
}
