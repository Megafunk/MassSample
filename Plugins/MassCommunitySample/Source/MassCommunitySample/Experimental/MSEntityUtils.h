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


USTRUCT()
struct FMSEntity
{
	GENERATED_BODY()
	
};