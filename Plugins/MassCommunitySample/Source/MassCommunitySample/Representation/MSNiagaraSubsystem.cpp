#include "MSNiagaraSubsystem.h"

#include "MSNiagaraActor.h"
#include "NiagaraComponent.h"
#include "Fragments/MSRepresentationFragments.h"


void UMSNiagaraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
		
	MassManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager().AsShared();
	
}

FSharedStruct UMSNiagaraSubsystem::GetOrCreateSharedNiagaraFragmentForSystemType(UNiagaraSystem* NiagaraSystem)
{

	//we only want to key these based off of unique types of niagara systems! Usually the entire fragment would be hashed.
	uint32 ParamsHash = PointerHash(NiagaraSystem);

	FSharedNiagaraSystemFragment SharedStructToReturn;

	//try to see if we have seen this system type before...
	if(PreexistingSharedNiagaraActors.Contains(ParamsHash))
	{
		//if yes, just grab the one we made earlier!
		return MassManager->GetOrCreateSharedFragmentByHash<FSharedNiagaraSystemFragment>(ParamsHash,SharedStructToReturn);
	}

	//if not, we need to spawn an entity+actor for it!
	AMSNiagaraActor* NewNiagaraActor = GetWorld()->SpawnActor<AMSNiagaraActor>();

	NewNiagaraActor->GetNiagaraComponent()->SetAsset(NiagaraSystem);
	
	SharedStructToReturn.NiagaraManagerActor = NewNiagaraActor;

	PreexistingSharedNiagaraActors.Add(ParamsHash,NewNiagaraActor);
	
	return MassManager->GetOrCreateSharedFragmentByHash<FSharedNiagaraSystemFragment>(ParamsHash,SharedStructToReturn);
	
}

