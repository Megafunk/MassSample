#include "MSNiagaraSubsystem.h"

#include "MassEntitySubsystem.h"
#include "MSNiagaraActor.h"
#include "NiagaraComponent.h"
#include "Fragments/MSRepresentationFragments.h"


void UMSNiagaraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
		
	MassManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager().AsShared();
	
}

FSharedStruct UMSNiagaraSubsystem::GetOrCreateSharedNiagaraFragmentForSystemType(UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride)
{

	// We only want to key these based off of unique types of niagara systems! Usually the entire fragment would be hashed.

	// Probably a more sensible way to get a consistent hash?
	uint32 NiagaraAssetHash = GetTypeHash(NiagaraSystem->GetPathName());
	uint32 ParamsHash = NiagaraAssetHash;
	if(StaticMeshOverride)
	{
		uint32 StaticMeshOverrideAssetHash = GetTypeHash(StaticMeshOverride->GetPathName());
		ParamsHash = HashCombineFast(NiagaraAssetHash,StaticMeshOverrideAssetHash);
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

	NewNiagaraActor->GetNiagaraComponent()->SetAsset(NiagaraSystem);

	if(StaticMeshOverride)
	{
		NewNiagaraActor->GetNiagaraComponent()->SetVariableStaticMesh("StaticMeshToRender", StaticMeshOverride);
	}
	SharedStructToReturn.NiagaraManagerActor = NewNiagaraActor;

	PreexistingSharedNiagaraActors.FindOrAdd(ParamsHash,NewNiagaraActor);
	
	return MassManager->GetOrCreateSharedFragmentByHash<FSharedNiagaraSystemFragment>(ParamsHash,SharedStructToReturn);
}

