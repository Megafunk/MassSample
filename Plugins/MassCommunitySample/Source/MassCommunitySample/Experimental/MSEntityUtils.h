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
		Template = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(*World, *MassEntityConfig);
	};

	FMassArchetypeHandle FinalizeTemplateArchetype(FMassEntityManager& EntityManager)
	{
		Template.SetArchetype(EntityManager.CreateArchetype(Template.GetCompositionDescriptor()));
		
		return Template.GetArchetype();

	};

	
	FMassEntityHandle SpawnEntity(FMassEntityManager& EntityManager) const
	{
		TArray<FMassEntityHandle> SpawnedEntities;
		EntityManager.BatchCreateEntities(Template.GetArchetype(), 1, SpawnedEntities);
		EntityManager.SetEntityFragmentsValues(SpawnedEntities[0], Template.GetInitialFragmentValues());
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