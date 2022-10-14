#pragma once
#include <ThirdParty/Vulkan/Include/vulkan/vulkan_core.h>

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationProcessor.h"
#include "MSNiagaraActor.h"
#include "MSNiagaraSubsystem.generated.h"

UCLASS()
class UMSNiagaraSubsystem : public UWorldSubsystem
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
	FSharedStruct GetOrCreateSharedNiagaraFragmentForSystemType(class UNiagaraSystem* NiagaraSystem);
	

	TMap<uint32, AMSNiagaraActor*> PreexistingSharedNiagaraActors;
};
