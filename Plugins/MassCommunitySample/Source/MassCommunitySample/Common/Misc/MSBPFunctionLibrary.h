// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityView.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MSBPFunctionLibrary.generated.h"


// 

// TODO @karl having two of these feels quite weird, it makes sense in C++ but do we care about the distinction in BP?
// Feedback please!! What makes the most sense? Personally I think BP stuff should be where we have the most freedom
// to just get data we need on the spot even if it means we do some repeated work. Or perhaps the mass entity view can be smarter?
/**
 * FMassEntityView wrapper for for general blueprint use
 * This can be rather evil due to the fact that the EntityView is transient in representing the actual state
 * If you want to store an entity ID longer term you might be better off with the FMSEntityHandleBPWrapper
 */
USTRUCT(BlueprintType)
struct FMSEntityViewBPWrapper
{
	GENERATED_BODY()

	FMSEntityViewBPWrapper() = default;

	FMSEntityViewBPWrapper(const FMassArchetypeHandle& Archetype, FMassEntityHandle EntityHandle)
	{
		EntityView = FMassEntityView(Archetype, EntityHandle);
	}
	
	FMSEntityViewBPWrapper(const FMassEntityManager& Manager, FMassEntityHandle EntityHandle)
	{
		EntityView = FMassEntityView(Manager,EntityHandle);
	}

	//This goofy function is needed due to protected functions in FMassEntityView preventing non template use :(
	FMassArchetypeHandle TempArchetypeGet(const FMassEntityManager& Manager) const
	{
		return Manager.GetArchetypeForEntity(EntityView.GetEntity());
	}

	FMassEntityView EntityView;

};

// An entity handle only BP wrapper.
USTRUCT(BlueprintType)
struct FMSEntityHandleBPWrapper
{
	GENERATED_BODY()
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
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass")
	static bool EntityHasFragment(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass",meta=(WorldContext = "WorldContextObject"))
	static bool EntityHasTag(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mass", meta=(WorldContext = "WorldContextObject", ExpandBoolAsExecs = "ReturnValue"))
	static bool IsEntityValid(FMSEntityViewBPWrapper Entity, UObject* WorldContextObject);
	
	
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FMSEntityViewBPWrapper SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig,
															 const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityTransform(const FMSEntityViewBPWrapper EntityHandle,const FTransform Transform, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetEntityTransform(FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass")
	static void SetEntityVelocity(FMSEntityViewBPWrapper EntityHandle, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "Mass")
	static void SetEntityForce(const FMSEntityViewBPWrapper EntityHandle, const FVector Force);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void DestroyEntity(const FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void FindHashGridEntitiesInBox(const FVector Center,const FVector Extents, TArray<FMSEntityViewBPWrapper>& Entities ,const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static void FindClosestHashGridEntityInBox(const FVector Center,const FVector Extents, FMSEntityViewBPWrapper& Entity, const UObject* WorldContextObject,EReturnSuccess& ReturnBranch);

	//TODO: Have this also helpfully add missing fragments
	/**
	 * Sets an entity's fragment data. This might need more testing...
	 */
	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject"))
	static void SetEntityFragment(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment,const UObject* WorldContextObject);


	/**
	 * You may need to make a new fragment struct variable to pass in here to serve as the type
	 * A better experience would probably require a custom k2 node thingy?
	 */
	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static FInstancedStruct GetEntityFragmentByType(FMSEntityViewBPWrapper Entity, FInstancedStruct Fragment,const UObject* WorldContextObject, EReturnSuccess& ReturnBranch);
	
};