// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassSpawnerSubsystem.h"
#include "MSSubsystem.h"
#include "VectorTypes.h"
#include "Common/Fragments/MSOctreeFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSBPFunctionLibrary)




bool UMSBPFunctionLibrary::EntityHasFragment(FMSEntityHandleBPWrapper EntityHandle, UScriptStruct* Fragment, UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!Fragment || !EntityManager)
	{
		return false;
	}
	
	if (!Fragment->IsChildOf(FMassFragment::StaticStruct())) 
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an non-fragment to to EntityHasFragment"));
		return false;	
	}
	
	if (EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to EntityHasFragment"));
	}
	
	FMassEntityView EntityView = FMassEntityView(*EntityManager, EntityHandle.EntityHandle);
	FStructView FragmentView = EntityView.GetFragmentDataStruct(Fragment);
	if (FragmentView.IsValid())
	{
		return true;
	}
	
	return false;
}

bool UMSBPFunctionLibrary::EntityHasTag(FMSEntityHandleBPWrapper EntityHandle, UScriptStruct* Tag, UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!Tag || !EntityManager)
	{
		return false;
	}
	
	if (!Tag->IsChildOf(FMassTag::StaticStruct()))
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an non-fragment to to EntityHasFragment"));
		return false;	
	}
	
	if (EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to EntityHasFragment"));
	}
	
	FMassEntityView EntityView = FMassEntityView(*EntityManager, EntityHandle.EntityHandle);
	if (EntityView.HasTag(*Tag))
	{
		return true;
	}
	
	return false;
}

bool UMSBPFunctionLibrary::IsEntityValid(FMSEntityHandleBPWrapper Entity, UObject* WorldContextObject)
{
	if (FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject))
	{
		return EntityManager->IsEntityValid(Entity.EntityHandle);
	}
	
	return false;
}

FMSEntityHandleBPWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig, const UObject* WorldContextObject,
                                                                         EReturnSuccess& ReturnBranch)
{
	ReturnBranch = EReturnSuccess::Failure;

	if (!MassEntityConfig)
	{
		return FMSEntityHandleBPWrapper();
	}
	
	const FMassEntityTemplate& EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(*WorldContextObject->GetWorld());

	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	auto SpawnerSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

	TArray<FMassEntityHandle> Entities;
	SpawnerSubsystem->SpawnEntities(EntityTemplate.GetTemplateID(), 1, FStructView(), TSubclassOf<UMassProcessor>(), Entities);

	//If no observers did anything, we can just assume the archetype is the same as our template
	
	FMSEntityHandleBPWrapper NewEntityWrapper;

	if (!Entities.IsEmpty()) {
		NewEntityWrapper.EntityHandle = Entities[0];
		ReturnBranch = EReturnSuccess::Success;
	}
	
	return NewEntityWrapper;
}

void UMSBPFunctionLibrary::SetEntityTransform(const FMSEntityHandleBPWrapper EntityHandle, const FTransform& Transform, UObject* WorldContextObject) {
	
	if (FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject)) {
		if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
		{
			UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to SetEntityTransform"));
			return;
		}		
		if (FTransformFragment* TransformFragmentPtr = EntityManager->GetFragmentDataPtr<FTransformFragment>(EntityHandle.EntityHandle)) 
		{
			*TransformFragmentPtr = FTransformFragment(Transform);
		}
	}
}

FTransform UMSBPFunctionLibrary::GetEntityTransform(const FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	if (FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject)) {
		if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
		{
			UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to GetEntityTransform"));
			return FTransform::Identity;
		}		
		
		if (const FTransformFragment* TransformFragmentPtr = EntityManager->GetFragmentDataPtr<FTransformFragment>(EntityHandle.EntityHandle))
		{
			return TransformFragmentPtr->GetTransform();
		}
	}
	
	return FTransform();
}

void UMSBPFunctionLibrary::SetEntityVelocity(const FMSEntityHandleBPWrapper EntityHandle, const FVector Velocity, const UObject* WorldContextObject)
{
	
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager)
	{
		return;
	}

	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		return;
	};

	if (auto MassFragmentPtr = EntityManager->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle.EntityHandle))
	{
		MassFragmentPtr->Value = Velocity;
	}
}

FVector UMSBPFunctionLibrary::GetEntityVelocity(const FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager)
	{
		return FVector::ZeroVector;
	}

	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		return FVector::ZeroVector;
	};

	if (const auto VelocityFragmentPtr = EntityManager->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle.EntityHandle))
	{
		return VelocityFragmentPtr->Value;
	}

	return FVector::ZeroVector;
}

void UMSBPFunctionLibrary::SetEntityForce(const FMSEntityHandleBPWrapper EntityHandle, const FVector Force, const UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager)
	{
		return;
	}

	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		return;
	};

	if (auto MassFragmentPtr = EntityManager->GetFragmentDataPtr<FMassForceFragment>(EntityHandle.EntityHandle))
	{
		MassFragmentPtr->Value = Force;
	}
}

void UMSBPFunctionLibrary::DestroyEntity(const FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager)
	{
		return;
	}

	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		return;
	};
	
	if (EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		EntityManager->DestroyEntity(EntityHandle.EntityHandle);
	}
	else
	{
		EntityManager->ReleaseReservedEntity(EntityHandle.EntityHandle);
	}
}

bool UMSBPFunctionLibrary::GetMassAgentEntity(FMSEntityHandleBPWrapper& OutEntity, UMassAgentComponent* Agent, const UObject* WorldContextObject)
{
	if (!Agent->IsEntityPendingCreation())
	{
		FMassEntityHandle EntityHandle = Agent->GetEntityHandle();
		OutEntity = FMSEntityHandleBPWrapper(EntityHandle);
		return true;
	}
	return false;
}

void UMSBPFunctionLibrary::FindOctreeEntitiesInBox(const FVector Center, const FVector Extents, TArray<FMSEntityHandleBPWrapper>& Entities,
                                                   const UObject* WorldContextObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if (UMSSubsystem* MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
		TArray<FMassEntityHandle> EntitiesFound;


		MassSampleSystem->MassSampleOctree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents), [&](const FMSEntityOctreeElement& OctreeElement)
		{
			EntitiesFound.Add(OctreeElement.EntityHandle);
		});

		for (auto EntityFound : EntitiesFound)
		{
			if (EntityManager.IsEntityValid(EntityFound))
			{
				Entities.Add(FMSEntityHandleBPWrapper(EntityFound));
			}
		}
	}
}

void UMSBPFunctionLibrary::FindClosestHashGridEntityInBox(const FVector Center, const FVector Extents, FMSEntityHandleBPWrapper& Entity,
                                                          const UObject* WorldContextObject, EReturnSuccess& ReturnBranch)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);
	
	ReturnBranch = EReturnSuccess::Failure;

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		double ShortestDistance = MAX_dbl;
		FMassEntityHandle EntityHandle;
		MassSampleSystem->MassSampleOctree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents), [&](const FMSEntityOctreeElement& OctreeElement)
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
			Entity = FMSEntityHandleBPWrapper(EntityHandle);
			ReturnBranch = EReturnSuccess::Success;
		}
	}
}


void UMSBPFunctionLibrary::SetEntityFragment(FMSEntityHandleBPWrapper EntityHandle, FInstancedStruct Fragment, const UObject* WorldContextObject)
{
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager) {
		return;
	}
	
	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to SetEntityFragment"));
		return;
	}		

	if (!Fragment.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid FInstancedStuct to SetEntityFragment"));
		return;
	}

	if (!Fragment.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.GetScriptStruct()->GetName());
		return;
	}

	FStructView StructView = EntityManager->GetFragmentDataStruct(EntityHandle.EntityHandle, Fragment.GetScriptStruct());

	// If it's not present just add it through the manager
	if (!StructView.IsValid())
	{
		EntityManager->AddFragmentInstanceListToEntity(EntityHandle.EntityHandle, {Fragment});
	}
	else
	{
		uint8* Memory = StructView.GetMemory();
		FMemory::Memcpy(Memory, Fragment.GetMutableMemory(), Fragment.GetScriptStruct()->GetStructureSize());
	}
}

FInstancedStruct UMSBPFunctionLibrary::GetEntityFragmentByType(FMSEntityHandleBPWrapper EntityHandle, FInstancedStruct Fragment,
                                                               const UObject* WorldContextObject, EReturnSuccess& ReturnBranch)
{
	ReturnBranch = EReturnSuccess::Failure;
	FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(WorldContextObject);
	if (!EntityManager) {
		return FInstancedStruct();
	}
	
	if (!Fragment.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid FInstancedStuct to GetEntityFragmentByType"));
		return FInstancedStruct();
	}

	if (!EntityManager->IsEntityValid(EntityHandle.EntityHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Passed in an invalid Entity to GetEntityFragmentByType"));
		return FInstancedStruct();
	}		

	if (!Fragment.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.GetScriptStruct()->GetName());
		return FInstancedStruct();
	}

	FStructView structview = EntityManager->GetFragmentDataStruct(EntityHandle.EntityHandle, Fragment.GetScriptStruct());

	if (structview.IsValid())
	{
		ReturnBranch = EReturnSuccess::Success;
		return FInstancedStruct(structview);
	}

	return FInstancedStruct();
}
