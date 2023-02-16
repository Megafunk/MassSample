#pragma once
#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "MSNiagaraActor.h"
#include "MSNiagaraSubsystem.generated.h"

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNiagaraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
protected:
	
	TSharedPtr<FMassEntityManager> MassManager;
	
	FMassArchetypeHandle BulletArchetype;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override
	{
		MassManager.Reset();
		PreexistingSharedNiagaraActors.Empty();
	};

public:
	// Creates/Finds a new niagara shared fragment and their manager actor. Can also set an overriden static mesh (a tad hacky though)
	FSharedStruct GetOrCreateSharedNiagaraFragmentForSystemType(class UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride);
	

	UPROPERTY()
	TMap<uint32, AMSNiagaraActor*> PreexistingSharedNiagaraActors;
};
