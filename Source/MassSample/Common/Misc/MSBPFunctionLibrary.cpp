// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"

#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassExecutor.h"
#include "MassGameplayDebugTypes.h"
#include "MassMovementFragments.h"
#include "MassProcessingPhase.h"
#include "MSDeferredCommands.h"
#include "MSSubsystem.h"
#include "AI/NavigationSystemBase.h"
#include "Common/Fragments/MSFragments.h"
#include "Experimental/MSEntityUtils.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"

FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig,
                                                                       const UObject* WorldContextObject,
                                                                       const bool bDebug)
{
	if (!MassEntityConfig) return FEntityHandleWrapper();


	UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	//todo: who should actually own an entity template? it's probably designed to have just one spawner own it?

	if (FMSEntitySpawnTemplate MassEntitySpawnData = FMSEntitySpawnTemplate(
		MassEntityConfig, WorldContextObject->GetWorld()))
	{
		if (bDebug)
		{
			MassEntitySpawnData.Template.GetMutableTags().Add<FMassSampleDebuggableTag>();
		}
		MassEntitySpawnData.Template.AddFragment_GetRef<FTransformFragment>().SetTransform(FTransform::Identity);
		MassEntitySpawnData.Template.AddFragment_GetRef<FTransformFragment>().GetMutableTransform().SetTranslation(
			FMath::VRand());

		// Finalize by actually
		// 1: creating/getting archetype
		MassEntitySpawnData.FinalizeTemplateArchetype(EntitySubSystem);
		// 2: spawning the entity
		FMassEntityHandle SpawnedEntity = MassEntitySpawnData.SpawnEntity(EntitySubSystem);


		return FEntityHandleWrapper{SpawnedEntity};
	}

	return FEntityHandleWrapper();
}


FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfigDeferred(
	AActor* Owner, UMassEntityConfigAsset* MassEntityConfig,
	const UObject* WorldContextObject)
{
	//	FMassExecutionContext ExecutionContext(WorldContextObject->GetWorld()->DeltaTimeSeconds);
	FMassExecutionContext ExecutionContext(0);

	ExecutionContext.SetDeferredCommandBuffer(MakeShareable(new FMassCommandBuffer()));


	if (!Owner || !MassEntityConfig) return FEntityHandleWrapper();

	if (const FMassEntityTemplate* EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*Owner, *MassEntityConfig))
	{
		auto EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		auto MassSampleSubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>();;

		const FMassEntityHandle ReservedEntity = EntitySubSystem->ReserveEntity();


		//todo: this is very slow! I am just doing this to be able to stuff configs in here for now
		TArray<const UScriptStruct*> FragmentTypesList;
		EntityTemplate->GetCompositionDescriptor().Fragments.ExportTypes(FragmentTypesList);
		TArray<const UScriptStruct*> TagsTypeList;

		EntityTemplate->GetCompositionDescriptor().Tags.ExportTypes(TagsTypeList);

		TArray<FInstancedStruct> InstanceStructs = TArray<FInstancedStruct>(FragmentTypesList);

		TConstArrayView<FInstancedStruct> InitialFragmentInstances = EntityTemplate->GetInitialFragmentValues();

		// InstanceStructs need to have the InitialFragmentInstances data-filled instanced structs, all of which already exist inside of InstanceStructs

		for (auto InitialFragmentInstance : InitialFragmentInstances)
		{
			int32 index = InstanceStructs.IndexOfByPredicate([&](const FInstancedStruct InstancedStructValue)
			{
				return InstancedStructValue.GetScriptStruct() == InitialFragmentInstance.GetScriptStruct();
			});
			if (index != INDEX_NONE)
				InstanceStructs[index] = InitialFragmentInstance;
		}

		ExecutionContext.Defer().PushCommand(
			FBuildEntityFromFragmentInstancesAndTags(ReservedEntity,
			                                         InstanceStructs,
			                                         TagsTypeList,
			                                         EntityTemplate->GetSharedFragmentValues()));


		ExecutionContext.FlushDeferred(*EntitySubSystem);
		//EntitySubSystem->FlushCommands(MakeShareable(new FMassCommandBuffer()));
		return FEntityHandleWrapper{ReservedEntity};
	}

	return FEntityHandleWrapper();
}




FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfigDeferredBugRepro(
	AActor* Owner, UMassEntityConfigAsset* MassEntityConfig,
	const UObject* WorldContextObject)
{
	
	if (!Owner || !MassEntityConfig) return FEntityHandleWrapper();

	if (const FMassEntityTemplate* EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*Owner, *MassEntityConfig))
	{
		auto EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

		const FMassEntityHandle ReservedEntity = EntitySubSystem->ReserveEntity();


		//todo: this is very slow! I am just doing this to be able to stuff configs in here for now
		TArray<const UScriptStruct*> FragmentTypesList;
		EntityTemplate->GetCompositionDescriptor().Fragments.ExportTypes(FragmentTypesList);
		TArray<const UScriptStruct*> TagsTypeList;

		EntityTemplate->GetCompositionDescriptor().Tags.ExportTypes(TagsTypeList);

		TArray<FInstancedStruct> InstanceStructs = TArray<FInstancedStruct>(FragmentTypesList);

		TConstArrayView<FInstancedStruct> InitialFragmentInstances = EntityTemplate->GetInitialFragmentValues();

		// InstanceStructs need to have the InitialFragmentInstances data-filled instanced structs, all of which already exist inside of InstanceStructs

		for (auto InitialFragmentInstance : InitialFragmentInstances)
		{
			int32 index = InstanceStructs.IndexOfByPredicate([&](const FInstancedStruct InstancedStructValue)
			{
				return InstancedStructValue.GetScriptStruct() == InitialFragmentInstance.GetScriptStruct();
			});
			if (index != INDEX_NONE)
				InstanceStructs[index] = InitialFragmentInstance;
		}
		// TODO: deferred BuildEntity can't seem to figure out representation? is it an initializer issue? 
		EntitySubSystem->Defer().PushCommand(
			FBuildEntityFromFragmentInstancesAndTags(ReservedEntity,
			                                         InstanceStructs,
			                                         TagsTypeList,
			                                         EntityTemplate->GetSharedFragmentValues()));
		
		EntitySubSystem->FlushCommands();
		//EntitySubSystem->FlushCommands(MakeShareable(new FMassCommandBuffer()));
		return FEntityHandleWrapper{ReservedEntity};
	}

	return FEntityHandleWrapper();
}





void UMSBPFunctionLibrary::SetEntityTransform(const FEntityHandleWrapper EntityHandle, const FTransform Transform,
                                              const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	check(EntitySubSystem)
	check(EntitySubSystem->IsEntityBuilt(EntityHandle.Entity))


	if (!EntitySubSystem->GetArchetypeComposition(EntitySubSystem->GetArchetypeForEntity(EntityHandle.Entity)).Fragments
	                    .Contains(*FTransformFragment::StaticStruct()))
		return;

	if (const auto TransformFragment = EntitySubSystem->GetFragmentDataPtr<FTransformFragment>(
		EntityHandle.Entity))
	{
		TransformFragment->SetTransform(Transform);
	}
}

void UMSBPFunctionLibrary::SetEntityCollisionQueryIgnoredActors(const FEntityHandleWrapper EntityHandle,
                                                                const TArray<AActor*> IgnoredActors,
                                                                const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	check(EntitySubSystem)


	if (const auto CollisionQueryFragment = EntitySubSystem->GetFragmentDataPtr<
		FLineTraceFragment>(EntityHandle.Entity))
	{
		CollisionQueryFragment->QueryParams.AddIgnoredActors(IgnoredActors);
	}
}

FTransform UMSBPFunctionLibrary::GetEntityTransform(const FEntityHandleWrapper EntityHandle,
                                                    const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	check(EntitySubSystem)

	if (!EntitySubSystem->IsEntityValid(EntityHandle.Entity)) return FTransform::Identity;

	if (const auto TransformFragmentPtr = EntitySubSystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle.Entity))
	{
		return TransformFragmentPtr->GetTransform();
	}

	return FTransform();
}

void UMSBPFunctionLibrary::SetEntityForce(const FEntityHandleWrapper EntityHandle, const FVector Force,
                                          const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	check(EntitySubSystem)
	if (!EntitySubSystem->GetArchetypeComposition(EntitySubSystem->GetArchetypeForEntity(EntityHandle.Entity)).Fragments
	                    .Contains(*FMassForceFragment::StaticStruct()))
		return;
	if (const auto MassForceFragmentPtr = EntitySubSystem->GetFragmentDataPtr<FMassForceFragment>(EntityHandle.Entity))
	{
		MassForceFragmentPtr->Value = Force;
	}
}


void UMSBPFunctionLibrary::FindHashGridEntitiesInSphere(const FVector Location, const double Radius,
                                                        TArray<FEntityHandleWrapper>& Entities,
                                                        const UObject* WorldContextObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		TArray<FMassEntityHandle> EntitiesFound;

		int32 numfound = MassSampleSystem->HashGrid.FindPointsInBall(Location, Radius,
             //todo-performance it feels bad to get random entities to query... 
             [&,Location](const FMassEntityHandle Entity)
             {
                const FVector EntityLocation = EntitySystem->GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
                return UE::Geometry::DistanceSquared(Location, EntityLocation);
             }, EntitiesFound);

		Entities.Reserve(numfound);
		for (auto EntityFound : EntitiesFound)
		{
			Entities.Add(FEntityHandleWrapper{EntityFound});
		}
	}
}

void UMSBPFunctionLibrary::FindClosestHashGridEntityInSphere(const FVector Location, const double Radius,
                                                             FEntityHandleWrapper& Entity,
                                                             const UObject* WorldContextObject,
                                                             TEnumAsByte<EReturnSuccess>& ReturnBranch)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

		const auto FoundEntityHashMember = MassSampleSystem->HashGrid.FindNearestInRadius(Location, Radius,
			//todo-performance it feels bad to get random entities to query... 
			[&,Location](const FMassEntityHandle Entity)
			{
				const FVector EntityLocation = EntitySystem->GetFragmentDataPtr<FTransformFragment>(Entity)->
				                                             GetTransform().GetLocation();
				return UE::Geometry::DistanceSquared(Location, EntityLocation);
			});

		if (FoundEntityHashMember.Key.IsValid())
		{
			Entity.Entity = FoundEntityHashMember.Key;
			ReturnBranch = EReturnSuccess::Success;
		}
		else
		{
			ReturnBranch = EReturnSuccess::Failure;
		}
	}
}

void UMSBPFunctionLibrary::AddFragmentToEntity(FStructViewBPWrapper Fragment, FEntityHandleWrapper Entity,
                                               const UObject* WorldContextObject)
{
	auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	EntitySystem->AddFragmentToEntity(Entity.Entity, Fragment.Struct.GetScriptStruct());
}

FString UMSBPFunctionLibrary::GetEntityDebugString(FEntityHandleWrapper Entity, const UObject* WorldContextObject)
{
	auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySystem)

	if (!Entity.Entity.IsValid())
		return FString();


	FStringOutputDevice OutPut;
	OutPut.SetAutoEmitLineTerminator(true);
	EntitySystem->DebugPrintEntity(Entity.Entity, OutPut);


	return FString{OutPut};
}
