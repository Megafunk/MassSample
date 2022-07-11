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
			                                                             const FVector EntityLocation = EntitySystem->
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


FString UMSBPFunctionLibrary::GetEntityDebugString(FEntityHandleWrapper Entity, const UObject* WorldContextObject)
{
	auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySystem)

	if (!Entity.Entity.IsValid())
		return FString();


	FStringOutputDevice OutPut;
	OutPut.SetAutoEmitLineTerminator(true);

#if WITH_EDITOR
	EntitySystem->DebugPrintEntity(Entity.Entity, OutPut);
#endif


	return FString{OutPut};
}

void UMSBPFunctionLibrary::SetEntityFragment(FEntityHandleWrapper Entity, FInstancedStructBPWrapper Fragment,
                                             const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySystem)

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

	FStructView structview = EntitySystem->GetFragmentDataStruct(Entity.Entity, Fragment.Struct.GetScriptStruct());

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

	const UMassEntitySubsystem* EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySystem)

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

	FStructView structview = EntitySystem->GetFragmentDataStruct(Entity.Entity, Fragment.Struct.GetScriptStruct());

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
