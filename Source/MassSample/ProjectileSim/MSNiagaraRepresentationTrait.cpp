// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraRepresentationTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MSProjectileFragments.h"
#include "NiagaraSystem.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{

	
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);

	BuildContext.AddFragment<FTransformFragment>();

	FSharedNiagaraSystemFragment SharedNiagaraSystemFragment;
	
	SharedNiagaraSystemFragment.NiagaraSystem = NiagaraSystemAsset;
	
	uint32 FragmentPropertiesHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(SharedNiagaraSystemFragment));
	
	FSharedStruct SharedFragment = EntitySubsystem->GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>(
		FragmentPropertiesHash, SharedNiagaraSystemFragment);
	
	BuildContext.AddSharedFragment(SharedFragment);
}
