// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityView.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MSBPFunctionLibrary.generated.h"


// An entity handle only BP wrapper.
USTRUCT(BlueprintType)
struct FMSEntityHandleBPWrapper
{
	GENERATED_BODY()

	FMSEntityHandleBPWrapper() = default;
	
	FMSEntityHandleBPWrapper(const FMassEntityHandle& InHandle) : EntityHandle(InHandle) {}
	
	FMassEntityHandle EntityHandle;
};

UENUM()
enum class EReturnSuccess : uint8
{
	Success,
	Failure
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	// Check if a given entity view has a fragment
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass", meta=(WorldContext = "WorldContextObject"))
	static bool EntityHasFragment(FMSEntityHandleBPWrapper Entity, UPARAM(meta=(MetaStruct="/Script/MassEntity.MassFragment")) UScriptStruct* Fragment, UObject* WorldContextObject);

	// Check if a given entity view has a tag
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass", meta=(WorldContext = "WorldContextObject"))
	static bool EntityHasTag(FMSEntityHandleBPWrapper Entity, UPARAM(meta=(MetaStruct="/Script/MassEntity.MassTag")) UScriptStruct* Tag, UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass", meta=(WorldContext = "WorldContextObject", ExpandBoolAsExecs = "ReturnValue"))
	static bool IsEntityValid(FMSEntityHandleBPWrapper Entity, UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static FMSEntityHandleBPWrapper SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig,
															 const UObject* WorldContextObject,EReturnSuccess& ReturnBranch);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityTransform(const FMSEntityHandleBPWrapper EntityHandle,const FTransform& Transform, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetEntityTransform(FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityVelocity(FMSEntityHandleBPWrapper EntityHandle, FVector Velocity, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FVector GetEntityVelocity(FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityForce(const FMSEntityHandleBPWrapper EntityHandle, const FVector Force, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void DestroyEntity(const FMSEntityHandleBPWrapper EntityHandle, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandBoolAsExecs = "ReturnValue"))
	static bool GetMassAgentEntity(FMSEntityHandleBPWrapper& OutEntity, UMassAgentComponent* Agent, const UObject* WorldContextObject);

	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void FindOctreeEntitiesInBox(const FVector Center,const FVector Extents, TArray<FMSEntityHandleBPWrapper>& Entities ,const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static void FindClosestHashGridEntityInBox(const FVector Center,const FVector Extents, FMSEntityHandleBPWrapper& Entity, const UObject* WorldContextObject,EReturnSuccess& ReturnBranch);

	/**
	 * Sets an entity's fragment data or adds it if it's ins't present. This might need more testing...
	 */
	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject"))
	static void SetEntityFragment(FMSEntityHandleBPWrapper Entity, FInstancedStruct Fragment,const UObject* WorldContextObject);


	/**
	 * You may need to make a new fragment struct variable to pass in here to serve as the type
	 * A better experience would probably require a custom k2 node thingy?
	 */
	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static FInstancedStruct GetEntityFragmentByType(FMSEntityHandleBPWrapper EntityHandle, FInstancedStruct Fragment,const UObject* WorldContextObject, EReturnSuccess& ReturnBranch);

	UFUNCTION(BlueprintPure)
	static void BreakIndexToInt(const FMSEntityHandleBPWrapper& InValue, int32& Index)
	{
		Index = InValue.EntityHandle.Index;
	};
	
};