#include "MSNiagaraSubsystem.h"

#include "MassEntitySubsystem.h"
#include "MSNiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Fragments/MSRepresentationFragments.h"


void UMSNiagaraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	auto MassSubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	
	MassManager = MassSubsystem->GetMutableEntityManager().AsShared();
	
}

FSharedStruct UMSNiagaraSubsystem::GetOrCreateSharedNiagaraFragmentForSystemType(UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride)
{

	// We only want to key these based off of unique types of niagara systems! Usually the entire fragment would be hashed.

	// Probably a more sensible way to get a consistent hash?
	uint32 NiagaraAssetHash = GetTypeHash(NiagaraSystem->GetPathName());
	uint32 ParamsHash = NiagaraAssetHash;
	if(StaticMeshOverride)
	{
		ParamsHash = HashCombineFast(NiagaraAssetHash,GetTypeHash(StaticMeshOverride->GetFName()));
	}
	if(MaterialOverride)
	{
		ParamsHash = HashCombineFast(NiagaraAssetHash,GetTypeHash(MaterialOverride->GetFName()));
	}
	FSharedNiagaraSystemFragment SharedStructToReturn;

	//try to see if we have seen this system type before...
	if(PreexistingSharedNiagaraActors.Contains(ParamsHash))
	{
		//if yes, just grab the one we made earlier!
		return MassManager->GetOrCreateSharedFragmentByHash<FSharedNiagaraSystemFragment>(ParamsHash,SharedStructToReturn);
	}

	FActorSpawnParameters SpawnParameters;

	SpawnParameters.ObjectFlags = RF_Transient | RF_DuplicateTransient;

	//if not, we need to spawn an entity+actor for it!
	AMSNiagaraActor* NewNiagaraActor = GetWorld()->SpawnActor<AMSNiagaraActor>(SpawnParameters);

	// We need this to tick last so that it receives the new gameplay state we create in the mass manager (stuff moving etc) for the next frame.
	NewNiagaraActor->GetNiagaraComponent()->SetTickBehavior(ENiagaraTickBehavior::ForceTickLast);
	NewNiagaraActor->GetNiagaraComponent()->SetAsset(NiagaraSystem);

	if(StaticMeshOverride)
	{
		NewNiagaraActor->GetNiagaraComponent()->SetVariableStaticMesh("StaticMeshToRender", StaticMeshOverride);

		if(MaterialOverride)
		{
			NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", MaterialOverride);
		}
		else
		{
			NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", StaticMeshOverride->GetMaterial(0));
		}
	}
	SharedStructToReturn.NiagaraManagerActor = NewNiagaraActor;

	PreexistingSharedNiagaraActors.FindOrAdd(ParamsHash,NewNiagaraActor);
	
	return MassManager->GetOrCreateSharedFragmentByHash<FSharedNiagaraSystemFragment>(ParamsHash,SharedStructToReturn);
}

