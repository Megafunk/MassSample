// Fill out your copyright notice in the Description page of Project Settings.
#include "MSNiagaraRepresentationTraits.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "Common/Fragments/MSFragments.h"
#include "Experimental/Physics/MSMassCollision.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "Representation/Fragments/MSRepresentationFragments.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// Evil main thread loads
	if(StaticMesh)
	{
		FConstSharedStruct SharedStaticMeshFragment = EntityManager.GetOrCreateConstSharedFragment(FMSSharedStaticMesh(StaticMesh));
		BuildContext.AddConstSharedFragment(SharedStaticMeshFragment);
	}
	StaticMesh.LoadSynchronous();
	SharedNiagaraSystem.LoadSynchronous();
	MaterialOverride.LoadSynchronous();


	
	UMSNiagaraSubsystem* NiagaraSubsystem = UWorld::GetSubsystem<UMSNiagaraSubsystem>(&World);

	BuildContext.RequireFragment<FTransformFragment>();

	UMaterial* Material = nullptr;

	if(MaterialOverride)
	{
	}

	FSharedStruct SharedFragment =
		NiagaraSubsystem->GetOrCreateSharedNiagaraFragmentForSystemType(SharedNiagaraSystem.Get(),StaticMesh.Get(),MaterialOverride.Get());

	BuildContext.AddSharedFragment(SharedFragment);
}
