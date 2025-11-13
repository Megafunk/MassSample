#pragma once

#include "MassRepresentationProcessor.h"
#include "MSNiagaraSubsystem.generated.h"


class AMSNiagaraActor;

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNiagaraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
protected:
	
	TSharedPtr<FMassEntityManager> MassManager = nullptr;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override
	{
		MassManager.Reset();
		PreexistingSharedNiagaraActors.Empty();
	};

public:
	// Creates/Finds a new niagara shared fragment and their manager actor. Can also set an overriden static mesh (a tad hacky though)
	AMSNiagaraActor* GetOrCreateNiagaraManagerForSystemType(class UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride = nullptr);
	
	UPROPERTY()
	TMap<uint32, TObjectPtr<AMSNiagaraActor>> PreexistingSharedNiagaraActors;
};
