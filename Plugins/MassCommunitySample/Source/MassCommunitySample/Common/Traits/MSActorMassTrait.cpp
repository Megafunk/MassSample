// Fill out your copyright notice in the Description page of Project Settings.


#include "MSActorMassTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSFragments.h"

void UMSActorMassTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	 BuildContext.AddFragment<FMassActorFragment>();
}

