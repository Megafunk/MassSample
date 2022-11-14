// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"

#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassExecutor.h"
#include "MassGameplayDebugTypes.h"
#include "MassMovementFragments.h"
#include "MSDeferredCommands.h"
#include "MSSubsystem.h"
#include "Common/Fragments/MSFragments.h"
#include "Experimental/MSEntityUtils.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"


FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(
    UMassEntityConfigAsset* MassEntityConfig,
	const UObject* WorldContextObject)
{
	if (!MassEntityConfig) return FEntityHandleWrapper();

	const FMassEntityTemplate& EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*WorldContextObject->GetWorld(), *MassEntityConfig);

	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                        GetMutableEntityManager();



	//todo: this is very slow! I am just doing this to be able to stuff configs in here for now
	TArray<const UScriptStruct*> FragmentTypesList;
	EntityTemplate.GetCompositionDescriptor().Fragments.ExportTypes(FragmentTypesList);
	TArray<const UScriptStruct*> TagsTypeList;

	EntityTemplate.GetCompositionDescriptor().Tags.ExportTypes(TagsTypeList);

	TArray<FInstancedStruct> InstanceStructs = TArray<FInstancedStruct>(FragmentTypesList);

	TConstArrayView<FInstancedStruct> InitialFragmentInstances = EntityTemplate.GetInitialFragmentValues();

	// InstanceStructs need to have the InitialFragmentInstances data-filled instanced structs, all of which already exist inside of InstanceStructs
	// FIXMEKARL: do we still need to do this in 5.1???

	for (auto InitialFragmentInstance : InitialFragmentInstances)
	{
		int32 index = InstanceStructs.IndexOfByPredicate([&](const FInstancedStruct InstancedStructValue)
		{
			return InstancedStructValue.GetScriptStruct() == InitialFragmentInstance.GetScriptStruct();
		});
		if (index != INDEX_NONE)
			InstanceStructs[index] = InitialFragmentInstance;
	}

	// Copy a new composition descriptor because it gets changed in the addcomposition call
	FMassArchetypeCompositionDescriptor CompositionDescriptor = EntityTemplate.GetCompositionDescriptor();

	// Reserve an entity
	const FMassEntityHandle ReservedEntity = EntityManager.ReserveEntity();

	// We are using a lambda here because we don't have a deferred command that can do  
	EntityManager.Defer().PushCommand<FMassDeferredCreateCommand>([&](FMassEntityManager& System)
	{
		EntityManager.BuildEntity(ReservedEntity,InstanceStructs,EntityTemplate.GetSharedFragmentValues());
		EntityManager.AddCompositionToEntity_GetDelta(ReservedEntity,CompositionDescriptor);
	});
	
	// Immediately flush? Doesn't seem too bad here but I imagine we could do this in a nicer way?
	EntityManager.FlushCommands();

	// trigger observers manually for now as I'm too lazy to use the batch add for now
	if (EntityManager.GetObserverManager().HasObserversForBitSet(EntityTemplate.GetCompositionDescriptor().Fragments, EMassObservedOperation::Add))
	{
		
		EntityManager.GetObserverManager().OnCompositionChanged(
			FMassArchetypeEntityCollection(EntityTemplate.GetArchetype(), {ReservedEntity},FMassArchetypeEntityCollection::NoDuplicates)
			, EntityTemplate.GetCompositionDescriptor()
			, EMassObservedOperation::Add);
	}


	return FEntityHandleWrapper{ReservedEntity};
}


void UMSBPFunctionLibrary::SetEntityTransform(const FEntityHandleWrapper EntityHandle, const FTransform Transform,
                                              const UObject* WorldContextObject)
{
	const FMassEntityManager& EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                                GetEntityManager();

	check(EntitySubSystem.IsEntityBuilt(EntityHandle.Entity))


	if (!EntitySubSystem.GetArchetypeComposition(EntitySubSystem.GetArchetypeForEntity(EntityHandle.Entity)).Fragments
	                    .Contains(*FTransformFragment::StaticStruct()))
		return;

	if (const auto TransformFragment = EntitySubSystem.GetFragmentDataPtr<FTransformFragment>(
		EntityHandle.Entity))
	{
		TransformFragment->SetTransform(Transform);
	}
}

void UMSBPFunctionLibrary::SetEntityCollisionQueryIgnoredActors(const FEntityHandleWrapper EntityHandle,
                                                                const TArray<AActor*> IgnoredActors,
                                                                const UObject* WorldContextObject)
{
	const FMassEntityManager& EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                                GetEntityManager();


	if (const auto CollisionQueryFragment = EntitySubSystem.GetFragmentDataPtr<
		FLineTraceFragment>(EntityHandle.Entity))
	{
		CollisionQueryFragment->QueryParams.AddIgnoredActors(IgnoredActors);
	}
}

FTransform UMSBPFunctionLibrary::GetEntityTransform(const FEntityHandleWrapper EntityHandle,
                                                    const UObject* WorldContextObject)
{
	const FMassEntityManager& EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                                GetEntityManager();


	if (!EntitySubSystem.IsEntityValid(EntityHandle.Entity)) return FTransform::Identity;

	if (const auto TransformFragmentPtr = EntitySubSystem.GetFragmentDataPtr<FTransformFragment>(EntityHandle.Entity))
	{
		return TransformFragmentPtr->GetTransform();
	}

	return FTransform();
}

void UMSBPFunctionLibrary::SetEntityForce(const FEntityHandleWrapper EntityHandle, const FVector Force,
                                          const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                              GetEntityManager();

	if (!EntityManager.GetArchetypeComposition(EntityManager.GetArchetypeForEntity(EntityHandle.Entity)).Fragments
	                  .Contains(*FMassForceFragment::StaticStruct()))
		return;
	if (const auto MassForceFragmentPtr = EntityManager.GetFragmentDataPtr<FMassForceFragment>(EntityHandle.Entity))
	{
		MassForceFragmentPtr->Value = Force;
	}
}

void UMSBPFunctionLibrary::DestroyEntity(const FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject)
{
	FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
																  GetMutableEntityManager();

	if(EntityHandle.Entity.IsValid())
	{
		EntityManager.DestroyEntity(EntityHandle.Entity);
	}
	else
	{
		EntityManager.ReleaseReservedEntity(EntityHandle.Entity);
	}


}


void UMSBPFunctionLibrary::FindHashGridEntitiesInSphere(const FVector Location, const double Radius,
                                                        TArray<FEntityHandleWrapper>& Entities,
                                                        const UObject* WorldContextObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
		                                                              GetEntityManager();
		TArray<FMassEntityHandle> EntitiesFound;

		int32 numfound = MassSampleSystem->HashGrid.FindPointsInBall(Location, Radius,
		                                                             //todo-performance it feels bad to get random entities to query... 
		                                                             [&,Location](const FMassEntityHandle Entity)
		                                                             {
			                                                             const FVector EntityLocation = EntityManager.
				                                                             GetFragmentDataPtr<FTransformFragment>(
					                                                             Entity)->GetTransform().GetLocation();
			                                                             return UE::Geometry::DistanceSquared(
				                                                             Location, EntityLocation);
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
                                                             EReturnSuccess& ReturnBranch)
{
	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);

	if (auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
		                                                              GetEntityManager();

		const auto FoundEntityHashMember = MassSampleSystem->HashGrid.FindNearestInRadius(Location, Radius,
			//todo-performance it feels bad to get random entities to query... 
			[&,Location](const FMassEntityHandle Entity)
			{
				const FVector EntityLocation = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity)->
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


void UMSBPFunctionLibrary::SetEntityFragment(FEntityHandleWrapper Entity, FInstancedStructBPWrapper Fragment,
                                             const UObject* WorldContextObject)
{
	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                              GetEntityManager();

	if (!Entity.Entity.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid Entity"));
		return;
	}
	if (!Fragment.Struct.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid FInstancedStructBPWrapper"));
		return;
	}

	if (!Fragment.Struct.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.Struct.GetScriptStruct()->GetName());
		return;
	}

	FStructView structview = EntityManager.GetFragmentDataStruct(Entity.Entity, Fragment.Struct.GetScriptStruct());

	if (structview.IsValid())
	{
		const auto memory = structview.GetMutableMemory();
		*memory = *Fragment.Struct.GetMemory();
	}
}


// Thanks to https://forums.unrealengine.com/t/tutorial-how-to-accept-wildcard-structs-in-your-ufunctions/18968/11?u=megafunk
// And also Jambax! (shamelessly stolen from: https://github.com/EpicGames/UnrealEngine/pull/9282/files)

// I recommend you do not rely on the existence of these functions too much in your projects as it will probably change a lot and be hard to replace in BP

#define LOCTEXT_NAMESPACE "UStructUtilsFunctionLibrary"


FInstancedStructBPWrapper UMSBPFunctionLibrary::GetEntityFragmentByType(FEntityHandleWrapper Entity,
                                                                        FInstancedStructBPWrapper Fragment,
                                                                        const UObject* WorldContextObject,
                                                                        EReturnSuccess& ReturnBranch)
{
	ReturnBranch = EReturnSuccess::Failure;

	const FMassEntityManager& EntityManager = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>()->
	                                                              GetEntityManager();

	if (!Entity.Entity.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid Entity"));
		return FInstancedStructBPWrapper();
	}
	if (!Fragment.Struct.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in an invalid FInstancedStructBPWrapper"));
		return FInstancedStructBPWrapper();
	}

	if (!Fragment.Struct.GetScriptStruct()->IsChildOf(FMassFragment::StaticStruct()))
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("Passed in a non fragment type to GetEntityFragmentByType: '%s'."),
		       *Fragment.Struct.GetScriptStruct()->GetName());
		return FInstancedStructBPWrapper();
	}

	FStructView structview = EntityManager.GetFragmentDataStruct(Entity.Entity, Fragment.Struct.GetScriptStruct());

	if (structview.IsValid())
	{
		ReturnBranch = EReturnSuccess::Success;
		return FInstancedStructBPWrapper{structview};
	}

	return FInstancedStructBPWrapper();
}

// Stubs to avoid linker errors
FInstancedStructBPWrapper UMSBPFunctionLibrary::MakeInstancedStruct(EReturnSuccess& ExecResult, const int32& Data)
{
	checkNoEntry();
	return {};
}

void BreakInstancedStruct(EReturnSuccess& ExecResult, const FInstancedStructBPWrapper& Fragment, int32& Data)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UMSBPFunctionLibrary::execMakeInstancedStruct)
{
	P_GET_ENUM_REF(EReturnSuccess, ExecResult);

	Stack.Step(Stack.Object, nullptr);

	const FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const uint8* StructData = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!StructProperty || !StructData)
	{
		FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::FatalError
		                                      , LOCTEXT("InstancedStruct_InvalidInputStructWarning",
		                                                "Data must be a valid Struct type"));

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);

		P_NATIVE_BEGIN;
			ExecResult = EReturnSuccess::Failure;
			(*(FInstancedStructBPWrapper*)RESULT_PARAM).Struct.Reset();
		P_NATIVE_END;
	}
	else
	{
		P_NATIVE_BEGIN;

			ExecResult = EReturnSuccess::Success;
			(*(FInstancedStructBPWrapper*)RESULT_PARAM).Struct.InitializeAs(StructProperty->Struct, StructData);
		P_NATIVE_END;
	}
}

DEFINE_FUNCTION(UMSBPFunctionLibrary::execBreakInstancedStruct)
{
	P_GET_ENUM_REF(EReturnSuccess, ExecResult);
	P_GET_STRUCT_REF(FInstancedStructBPWrapper, InstancedStruct);

	Stack.StepCompiledIn<FProperty>(nullptr);
	const FProperty* ValueProp = Stack.MostRecentProperty;
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	ExecResult = EReturnSuccess::Failure;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::FatalError,
			LOCTEXT("InstancedStruct_InvalidStructWarning",
			        "Failed to resolve the data type for Break Instanced Struct")
		);

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}

	const FStructProperty* OutputStructProp = CastField<FStructProperty>(ValueProp);
	if (!OutputStructProp)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::NonFatalError,
			LOCTEXT("InstancedStruct_InvalidOutputStructWarning", "Output must be a valid Struct type"));

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else
	{
		P_NATIVE_BEGIN;
			if (InstancedStruct.Struct.IsValid() && InstancedStruct.Struct.GetScriptStruct()->IsChildOf(
				OutputStructProp->Struct))
			{
				OutputStructProp->Struct->CopyScriptStruct(ValuePtr, InstancedStruct.Struct.GetMemory());
				ExecResult = EReturnSuccess::Success;
			}
			else
			{
				ExecResult = EReturnSuccess::Failure;
			}
		P_NATIVE_END;
	}
}

#undef LOCTEXT_NAMESPACE
