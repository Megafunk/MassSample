// Fill out your copyright notice in the Description page of Project Settings.
#include "MSNiagaraRepresentationTraits.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "Common/Fragments/MSFragments.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "Representation/Fragments/MSRepresentationFragments.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	UMSNiagaraSubsystem* ProjectileSubsystem = UWorld::GetSubsystem<UMSNiagaraSubsystem>(&World);

	BuildContext.AddFragment<FTransformFragment>();

	FSharedStruct SharedFragment = ProjectileSubsystem->GetOrCreateSharedNiagaraFragmentForSystemType(
		SharedNiagaraSystem);

	BuildContext.AddSharedFragment(SharedFragment);
}
