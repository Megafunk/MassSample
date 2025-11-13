// Fill out your copyright notice in the Description page of Project Settings.
#include "MSNiagaraRepresentationTraits.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "Common/Fragments/MSFragments.h"
#include "Experimental/Physics/MSMassCollision.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "Representation/Fragments/MSRepresentationFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSNiagaraRepresentationTraits)

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// Evil main thread loads
	StaticMesh.LoadSynchronous();
	SharedNiagaraSystem.LoadSynchronous();
	MaterialOverride.LoadSynchronous();

	if(StaticMesh)
	{
		FConstSharedStruct SharedStaticMeshFragment = EntityManager.GetOrCreateConstSharedFragment(FMSSharedStaticMesh(StaticMesh));
		BuildContext.AddConstSharedFragment(SharedStaticMeshFragment);
	}
	
	BuildContext.RequireFragment<FTransformFragment>();
	
	// @todo support material overrides
	if(MaterialOverride)
	{
	}

	if (!BuildContext.IsInspectingData()) 
	{
		UMSNiagaraSubsystem* NiagaraSubsystem = UWorld::GetSubsystem<UMSNiagaraSubsystem>(&World);
		if (ensure(NiagaraSubsystem)) 
		{
			AMSNiagaraActor* NiagaraManagerActor = NiagaraSubsystem->GetOrCreateNiagaraManagerForSystemType(
				SharedNiagaraSystem.Get(),
				StaticMesh.Get(),
				MaterialOverride.Get());

			FMSSharedNiagaraSystemFragment SharedNiagaraSystemFragment;
			SharedNiagaraSystemFragment.NiagaraManagerActor = NiagaraManagerActor;
			const FSharedStruct& SharedFragment = EntityManager.GetOrCreateSharedFragment(SharedNiagaraSystemFragment);
			BuildContext.AddSharedFragment(SharedFragment);
		}
	}

}
