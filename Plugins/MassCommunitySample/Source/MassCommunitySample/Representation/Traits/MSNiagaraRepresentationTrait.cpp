// Fill out your copyright notice in the Description page of Project Settings.
#include "MSNiagaraRepresentationTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSFragments.h"
#include "Representation/MSNiagaraSubsystem.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMSNiagaraSubsystem* ProjectileSubsystem = UWorld::GetSubsystem<UMSNiagaraSubsystem>(&World);

	BuildContext.AddFragment<FTransformFragment>();


	FSharedStruct SharedFragment = ProjectileSubsystem->GetOrCreateSharedNiagaraFragmentForSystemType(
		SharedNiagaraSystem);

	BuildContext.AddSharedFragment(SharedFragment);
}

void UMSNiagaraRepresentationTrait::ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	if (!SharedNiagaraSystem)
	{
		UE_VLOG(&World, LogMass, Error, TEXT("Failed to get SharedNiagaraSystem."));
		return;
	}
}
