// Fill out your copyright notice in the Description page of Project Settings.


#include "MSActorMassTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSActorMassTrait)

void UMSActorMassTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	 BuildContext.AddFragment<FMassActorFragment>();
}

