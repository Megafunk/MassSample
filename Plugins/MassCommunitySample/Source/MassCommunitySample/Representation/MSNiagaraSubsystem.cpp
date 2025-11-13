
#include "MSNiagaraSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MSNiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Fragments/MSRepresentationFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSNiagaraSubsystem)


void UMSNiagaraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	auto MassSubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	
	MassManager = MassSubsystem->GetMutableEntityManager().AsShared();
	
}

AMSNiagaraActor* UMSNiagaraSubsystem::GetOrCreateNiagaraManagerForSystemType(UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride)
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
	FMSSharedNiagaraSystemFragment SharedStructToReturn;

	// try to see if we have made a manager for hash before...
	// Note that this TObjectPtr is a pointer to a pointer
	if(TObjectPtr<AMSNiagaraActor>* ExistingManagerActor = PreexistingSharedNiagaraActors.Find(ParamsHash))
	{
		//if yes, just grab the one we made earlier!
		return *ExistingManagerActor;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags = RF_Transient;
	SpawnParameters.bNoFail = true;

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
	
	return NewNiagaraActor;
}

