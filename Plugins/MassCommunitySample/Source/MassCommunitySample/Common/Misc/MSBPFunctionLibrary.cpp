// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"
#include "CoreMinimal.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassMovementFragments.h"
#include "MassSpawnerSubsystem.h"
#include "MSSubsystem.h"
#include "VectorTypes.h"
#include "Common/Fragments/MSOctreeFragments.h"


bool UMSBPFunctionLibrary::EntityHasFragment(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment)
{
	if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		return Entity.EntityView.GetFragmentDataStruct(Fragment.GetScriptStruct()).IsValid();
	}

	return false;
}

bool UMSBPFunctionLibrary::EntityHasTag(FMSEntityViewBPWrapper Entity, FInstancedStruct Tag, UObject* WorldContextObject)
{
	if (Tag.IsValid() && Tag.GetScriptStruct()->IsChildOf(FMassTag::StaticStruct()))
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

		return EntityManager.GetArchetypeComposition(Entity.TempArchetypeGet(EntityManager)).Tags.Contains(*Tag.GetScriptStruct());
	}

	return false;
}

bool UMSBPFunctionLibrary::IsEntityValid(FMSEntityViewBPWrapper Entity, UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	return EntityManager.IsEntityValid(Entity.EntityView.GetEntity());
}

FMSEntityViewBPWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig, const UObject* WorldContextObject,
                                                                         EReturnSuccess& ReturnBranch)
{
	if (!MassEntityConfig)
	{
		ReturnBranch = EReturnSuccess::Failure;
		return FMSEntityViewBPWrapper();
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 3
	const FMassEntityTemplate& EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(*WorldContextObject->GetWorld(),*WorldContextObject->GetWorld());
#else
	const FMassEntityTemplate& EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(*WorldContextObject->GetWorld());
#endif

	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	auto SpawnerSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

	TArray<FMassEntityHandle> Entities;
	SpawnerSubsystem->SpawnEntities(EntityTemplate.GetTemplateID(), 1, FStructView(), TSubclassOf<UMassProcessor>(), Entities);

	//If no observers did anything, we can just assume the archetype is the same as our template
	FMSEntityViewBPWrapper NewEntityWrapper;
	NewEntityWrapper.EntityView = FMassEntityView(EntityManager, Entities[0]);

	ReturnBranch = EReturnSuccess::Success;

	return NewEntityWrapper;
}


void UMSBPFunctionLibrary::SetEntityTransform(const FMSEntityViewBPWrapper EntityHandle, const FTransform Transform)
{
	if (!EntityHandle.EntityView.GetEntity().IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity"));
		return;
	}

	if (const auto TransformFragment = EntityHandle.EntityView.GetFragmentDataPtr<FTransformFragment>())
	{
		TransformFragment->SetTransform(Transform);
	}
}


FTransform UMSBPFunctionLibrary::GetEntityTransform(const FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();


	if (!EntityManager.IsEntityValid(EntityHandle.EntityView.GetEntity())) return FTransform::Identity;

	if (const auto TransformFragmentPtr = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle.EntityView.GetEntity()))
	{
		return TransformFragmentPtr->GetTransform();
	}

	return FTransform();
}

void UMSBPFunctionLibrary::SetEntityVelocity(const FMSEntityViewBPWrapper EntityHandle, const FVector Velocity)
{
	if (!EntityHandle.EntityView.GetEntity().IsValid())
	{
		return;
	};

	if (auto MassFragmentPtr = EntityHandle.EntityView.GetFragmentDataPtr<FMassVelocityFragment>())
	{
		MassFragmentPtr->Value = Velocity;
	}
}

FVector UMSBPFunctionLibrary::GetEntityVelocity(const FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();


	if (!EntityManager.IsEntityValid(EntityHandle.EntityView.GetEntity())) return FVector();

	if (const auto VelocityFragmentPtr = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle.EntityView.GetEntity()))
	{
		return VelocityFragmentPtr->Value;
	}

	return FVector();
}

void UMSBPFunctionLibrary::SetEntityForce(const FMSEntityViewBPWrapper EntityHandle, const FVector Force)
{
	if (!EntityHandle.EntityView.GetEntity().IsValid())
	{
		return;
	};

	if (auto MassFragmentPtr = EntityHandle.EntityView.GetFragmentDataPtr<FMassForceFragment>())
	{
		MassFragmentPtr->Value = Force;
	}
}

void UMSBPFunctionLibrary::DestroyEntity(const FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	if (EntityHandle.EntityView.GetEntity().IsValid())
	{
		EntityManager.DestroyEntity(EntityHandle.EntityView.GetEntity());
	}
	else
	{
		EntityManager.ReleaseReservedEntity(EntityHandle.EntityView.GetEntity());
	}
}

bool UMSBPFunctionLibrary::GetMassAgentEntity(FMSEntityViewBPWrapper& OutEntity, UMassAgentComponent* Agent, const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	if (!Agent->IsEntityPendingCreation())
	{
		FMassEntityHandle EntityHandle = Agent->GetEntityHandle();
		OutEntity = FMSEntityViewBPWrapper(EntityManager, EntityHandle);
		return true;
	}
	return false;
}

void UMSBPFunctionLibrary::FindOctreeEntitiesInBox(const FVector Center, const FVector Extents, TArray<FMSEntityViewBPWrapper>& Entities,
                                                   const UObject* WorldContextObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
		TArray<FMassEntityHandle> EntitiesFound;


		MassSampleSystem->Octree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents), [&](const FMSEntityOctreeElement& OctreeElement)
		{
			EntitiesFound.Add(OctreeElement.EntityHandle);
		});

		for (auto EntityFound : EntitiesFound)
		{
			if (EntityManager.IsEntityValid(EntityFound))
			{
				Entities.Add(FMSEntityViewBPWrapper(EntityManager, EntityFound));
			}
		}
	}
}

void UMSBPFunctionLibrary::FindClosestHashGridEntityInBox(const FVector Center, const FVector Extents, FMSEntityViewBPWrapper& Entity,
                                                          const UObject* WorldContextObject, EReturnSuccess& ReturnBranch)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		double ShortestDistance = MAX_dbl;
		FMassEntityHandle EntityHandle;
		MassSampleSystem->Octree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents), [&](const FMSEntityOctreeElement& OctreeElement)
		{
			double Distance = UE::Geometry::DistanceSquared(FVector(OctreeElement.Bounds.Center), Center);

			if (Distance < ShortestDistance)
			{
				EntityHandle = OctreeElement.EntityHandle;
				ShortestDistance = Distance;
			}
		});
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

		if (EntityHandle.IsValid() && ShortestDistance != MAX_dbl)
		{
			Entity = FMSEntityViewBPWrapper(EntityManager, EntityHandle);

			ReturnBranch = EReturnSuccess::Success;
		}
		else
		{
			ReturnBranch = EReturnSuccess::Failure;
		}
	}
}


void UMSBPFunctionLibrary::SetEntityFragment(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment, const UObject* WorldContextObject)
{
	auto& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	if (!Entity.EntityView.GetEntity().IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid Entity"));
		return;
	}
	if (!Fragment.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid FInstancedStuct"));
		return;
	}

	if (!Fragment.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.GetScriptStruct()->GetName());
		return;
	}

	FStructView StructView = EntityManager.GetFragmentDataStruct(Entity.EntityView.GetEntity(), Fragment.GetScriptStruct());

	// If it's not present just add it through the manager
	if (!StructView.IsValid())
	{
		EntityManager.AddFragmentInstanceListToEntity(Entity.EntityView.GetEntity(), {Fragment});
	}
	else
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 3
		auto Memory = StructView.GetMutableMemory();
		FMemory::Memcpy(Memory,Fragment.GetMutableMemory(),Fragment.GetScriptStruct()->GetStructureSize());
#else
		uint8* Memory = StructView.GetMemory();
		FMemory::Memcpy(Memory, Fragment.GetMutableMemory(), Fragment.GetScriptStruct()->GetStructureSize());
#endif
	}
}

FInstancedStruct UMSBPFunctionLibrary::GetEntityFragmentByType(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment,
                                                               const UObject* WorldContextObject, EReturnSuccess& ReturnBranch)
{
	ReturnBranch = EReturnSuccess::Failure;

	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	if (!Entity.EntityView.GetEntity().IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid Entity"));
		return FInstancedStruct();
	}
	if (!Fragment.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid FInstancedStuct"));
		return FInstancedStruct();
	}

	if (!Fragment.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.GetScriptStruct()->GetName());
		return FInstancedStruct();
	}

	FStructView structview = EntityManager.GetFragmentDataStruct(Entity.EntityView.GetEntity(), Fragment.GetScriptStruct());

	if (structview.IsValid())
	{
		ReturnBranch = EReturnSuccess::Success;
		return FInstancedStruct(structview);
	}

	return FInstancedStruct();
}
