// Fill out your copyright notice in the Description page of Project Settings.

#include "MSBasicVisualizationTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassActorSubsystem.h"

UMSBasicVisualizationTrait::UMSBasicVisualizationTrait()
{

	
	Params.LODRepresentation[EMassLOD::High] = EMassRepresentationType::StaticMeshInstance;
	Params.LODRepresentation[EMassLOD::Medium] = EMassRepresentationType::StaticMeshInstance;
	Params.LODRepresentation[EMassLOD::Low] = EMassRepresentationType::StaticMeshInstance;
	Params.LODRepresentation[EMassLOD::Off] = EMassRepresentationType::StaticMeshInstance;


}


void UMSBasicVisualizationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	Super::BuildTemplate(BuildContext, World);
	
	BuildContext.AddFragment<FMassActorFragment>();

	BuildContext.AddTag<FBasicVisualizationTag>();

}
