#pragma once
#include <ThirdParty/Vulkan/Include/vulkan/vulkan_core.h>

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationProcessor.h"
#include "MSNiagaraActor.h"
#include "EntitySystem/MovieSceneEntityIDs.h"

#include "Field/FieldSystemNodes.h"
#include "MSNiagaraSubsystem.generated.h"

UCLASS()
class UMSNiagaraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
protected:
	
	UPROPERTY()
	UMassEntitySubsystem* MassSubsystem;
	
	FMassArchetypeHandle BulletArchetype;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	FSharedStruct GetOrCreateSharedNiagaraFragmentForSystemType(class UNiagaraSystem* NiagaraSystem);

	TMap<uint32, AMSNiagaraActor*> PreexistingSharedNiagaraActors;
};
