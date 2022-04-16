// Fill out your copyright notice in the Description page of Project Settings.


#include "MSHashGridTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "Fragments/MSHashGridFragments.h"

void UMSHashGridTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{

	BuildContext.AddFragment<FMSGridCellStartingLocationFragment>();
	BuildContext.AddTag<FMSInHashGridTag>();

}
