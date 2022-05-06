// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplate.h"
#include "GameFramework/GameStateBase.h"
#include "MSEntityUtils.generated.h"

/**
 * 
 */


USTRUCT()
struct FMSEntitySpawnTemplate
{
	GENERATED_BODY()

	FMSEntitySpawnTemplate() = default;


	FMSEntitySpawnTemplate(const UMassEntityConfigAsset* MassEntityConfig, const UWorld* World)
	{
		AGameStateBase* GameState = World->GetGameState();
		Template = *MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(*GameState, *MassEntityConfig);
	};

	FMassArchetypeHandle FinalizeTemplateArchetype(UMassEntitySubsystem* EntitySubSystem)
	{
		Template.SetArchetype(EntitySubSystem->CreateArchetype(Template.GetCompositionDescriptor(),Template.GetSharedFragmentValues()));
		
		return Template.GetArchetype();

	};

	
	FMassEntityHandle SpawnEntity(UMassEntitySubsystem* EntitySubSystem) const
	{
		TArray<FMassEntityHandle> SpawnedEntities;
		EntitySubSystem->BatchCreateEntities(Template.GetArchetype(), 1, SpawnedEntities);
		EntitySubSystem->SetEntityFragmentsValues(SpawnedEntities[0], Template.GetInitialFragmentValues());
		return  SpawnedEntities[0];
	};

	operator bool() const { return Template.IsValid(); }
	
	FMassEntityTemplate Template;
	
};




// FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(AActor* Owner, UMassEntityConfigAsset* MassEntityConfig,
// 																		const UObject* WorldContextObject)
// {
//
// 	if (!Owner || !MassEntityConfig) return FEntityHandleWrapper();
// 	
// 	//todo: who should actually own an entity template? it's probably designed to have just one spawner own it?
// 	if(const FMassEntityTemplate* EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
// 		*Owner, *MassEntityConfig))
// 	{
// 		UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
//
// 	
// 		TArray<FMassEntityHandle> SpawnedEntities;
// 		EntityTemplate->GetMutableTags().Add<FMassDebuggableTag>();
// 		//Using batch create to trigger observers 
// 		EntitySubSystem->BatchCreateEntities(EntityTemplate->GetArchetype(), 1, SpawnedEntities);
//
// 		const TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate->GetInitialFragmentValues();
//
// 		
// 		EntitySubSystem->SetEntityFragmentsValues(SpawnedEntities[0], FragmentInstances);
// 		
// 		return  FEntityHandleWrapper{SpawnedEntities[0]};
// 	}
// 	
// 	return FEntityHandleWrapper();
// }