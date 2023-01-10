// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"

#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassExecutor.h"
#include "MassMovementFragments.h"
#include "MSDeferredCommands.h"
#include "MSSubsystem.h"
#include "VectorTypes.h"
#include "Common/Fragments/MSOctreeFragments.h"
#include "Experimental/MSEntityUtils.h"


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
		const FMassEntityManager& EntityManager =
			WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

		return EntityManager.GetArchetypeComposition(Entity.TempArchetypeGet(EntityManager)).Tags.Contains(
			*Tag.GetScriptStruct());
	}

	return false;
}

bool UMSBPFunctionLibrary::IsEntityValid(FMSEntityViewBPWrapper Entity, UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	return EntityManager.IsEntityValid(Entity.EntityView.GetEntity());
}

FMSEntityViewBPWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig,
	const UObject* WorldContextObject, EReturnSuccess& ReturnBranch)
{
	if (!MassEntityConfig)
	{
		ReturnBranch = EReturnSuccess::Failure;
		return FMSEntityViewBPWrapper();

	}

	const FMassEntityTemplate& EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*WorldContextObject->GetWorld(), *MassEntityConfig);

	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();


	//todo: this is very slow! I am just doing this to be able to stuff configs in here for now
	TArray<const UScriptStruct*> FragmentTypesList;
	EntityTemplate.GetCompositionDescriptor().Fragments.ExportTypes(FragmentTypesList);
	TArray<const UScriptStruct*> TagsTypeList;

	EntityTemplate.GetCompositionDescriptor().Tags.ExportTypes(TagsTypeList);

	TArray<FInstancedStruct> InstanceStructs = TArray<FInstancedStruct>(FragmentTypesList);


	// Copy a new composition descriptor because it gets changed in the addcomposition call
	FMassArchetypeCompositionDescriptor CompositionDescriptor = EntityTemplate.GetCompositionDescriptor();

	// Reserve an entity from the manager
	const FMassEntityHandle ReservedEntity = EntityManager.ReserveEntity();

	// We are using a lambda here because we don't have a deferred command that can do 
	EntityManager.Defer().PushCommand<FMassDeferredCreateCommand>([&](FMassEntityManager& System)
	{
		EntityManager.BuildEntity(ReservedEntity, InstanceStructs, EntityTemplate.GetSharedFragmentValues());
		// get the leftover chunk fragments and tags not done in BuildEntity..
		EntityManager.AddCompositionToEntity_GetDelta(ReservedEntity, CompositionDescriptor);
	});

	// Immediately flush? Doesn't seem too bad here but I imagine we could do this in a nicer way?
	// Could maybe have an optional defer bool or something?
	EntityManager.FlushCommands();

	// trigger observers manually for now as using the batch add feels off? Might be different now as of 5.1?
	if (EntityManager.GetObserverManager().OnCompositionChanged(
										FMassArchetypeEntityCollection(EntityTemplate.GetArchetype(), {ReservedEntity},
										FMassArchetypeEntityCollection::NoDuplicates)
		, EntityTemplate.GetCompositionDescriptor()
		, EMassObservedOperation::Add))
	{
		// If this is true, observers might have changed the archetype of our new entity
		// so we ask the manager for the archetype again (constructing a new entity view)
		return FMSEntityViewBPWrapper(EntityManager, ReservedEntity);
	}

	//If no observers did anything, we can just assume the archetype is the same as our template
	FMSEntityViewBPWrapper NewEntityWrapper;
	NewEntityWrapper.EntityView = FMassEntityView(EntityTemplate.GetArchetype(), ReservedEntity);

	ReturnBranch = EReturnSuccess::Success;

	return NewEntityWrapper;


	// Even though we are pretty certain what the composition here the possibility of 
}


void UMSBPFunctionLibrary::SetEntityTransform(const FMSEntityViewBPWrapper EntityHandle, const FTransform Transform, const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	if (!EntityManager.IsEntityValid(EntityHandle.EntityView.GetEntity()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid Entity"));
		return;
	}


	if (!EntityManager.GetArchetypeComposition(EntityManager.GetArchetypeForEntity(EntityHandle.EntityView.GetEntity())).Fragments
	                  .Contains(*FTransformFragment::StaticStruct()))
		return;

	if (const auto TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(
		EntityHandle.EntityView.GetEntity()))
	{
		TransformFragment->SetTransform(Transform);
	}
}


FTransform UMSBPFunctionLibrary::GetEntityTransform(const FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();


	if (!EntityManager.IsEntityValid(EntityHandle.EntityView.GetEntity())) return FTransform::Identity;

	if (const auto TransformFragmentPtr = EntityManager.GetFragmentDataPtr<FTransformFragment>(
		EntityHandle.EntityView.GetEntity()))
	{
		return TransformFragmentPtr->GetTransform();
	}

	return FTransform();
}

void UMSBPFunctionLibrary::SetEntityVelocity(const FMSEntityViewBPWrapper EntityHandle, const FVector Velocity)
{
	if (auto MassFragmentPtr = EntityHandle.EntityView.GetFragmentDataPtr<FMassVelocityFragment>())
	{
		MassFragmentPtr->Value = Velocity;
	}
}
void UMSBPFunctionLibrary::SetEntityForce(const FMSEntityViewBPWrapper EntityHandle, const FVector Force)
{
	
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


void UMSBPFunctionLibrary::FindOctreeEntitiesInBox(const FVector Center, const FVector Extents, TArray<FMSEntityViewBPWrapper>& Entities,
                                                     const UObject* WorldContextObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
		TArray<FMassEntityHandle> EntitiesFound;


		MassSampleSystem->Octree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents),
         [&](const FMSEntityOctreeElement& OctreeElement)
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
                                                          const UObject* WorldContextObject,
                                                          EReturnSuccess& ReturnBranch)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		double ShortestDistance = MAX_dbl;
		FMassEntityHandle EntityHandle;
		MassSampleSystem->Octree2.FindElementsWithBoundsTest(FBoxCenterAndExtent(Center, Extents),
             [&](const FMSEntityOctreeElement& OctreeElement)
             {
                 double Distance = UE::Geometry::DistanceSquared(
                     FVector(OctreeElement.Bounds.Center), Center);

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


void UMSBPFunctionLibrary::SetEntityFragment(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment,
                                             const UObject* WorldContextObject)
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

	FStructView structview = EntityManager.GetFragmentDataStruct(Entity.EntityView.GetEntity(),
	                                                             Fragment.GetScriptStruct());
	
	if (structview.IsValid())
	{
		auto Memory = structview.GetMutableMemory();
		*Memory = *Fragment.GetMemory();
	}
}

FInstancedStruct UMSBPFunctionLibrary::GetEntityFragmentByType(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment, const UObject* WorldContextObject,
                                                               EReturnSuccess& ReturnBranch)
{
	ReturnBranch = EReturnSuccess::Failure;

	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                              GetEntityManager();

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

	FStructView structview = EntityManager.GetFragmentDataStruct(Entity.EntityView.GetEntity(),
	                                                             Fragment.GetScriptStruct());

	if (structview.IsValid())
	{
		ReturnBranch = EReturnSuccess::Success;
		return structview;
	}

	return FInstancedStruct();
}
