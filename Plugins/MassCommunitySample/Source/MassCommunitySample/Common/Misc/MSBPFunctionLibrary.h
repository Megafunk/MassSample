// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"

#include "MassEntityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "MSBPFunctionLibrary.generated.h"



USTRUCT(BlueprintType)
struct FEntityHandleWrapper
{
	GENERATED_BODY()
	FMassEntityHandle Entity;

};
// A fragment wrapped inside an instanced struct... which is also wrapped inside a blueprint wrapper! It sucks but it's 
USTRUCT(BlueprintType)
struct FInstancedStructBPWrapper
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, meta = (BaseStruct = "MassFragment", ExcludeBaseStruct))
	FInstancedStruct Struct;
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



	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FEntityHandleWrapper SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig,
															 const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityTransform(const FEntityHandleWrapper EntityHandle,const FTransform Transform, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityCollisionQueryIgnoredActors(const FEntityHandleWrapper EntityHandle,const TArray<AActor*> IgnoredActors,const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetEntityTransform(FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityForce(FEntityHandleWrapper EntityHandle, FVector Force, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void DestroyEntity(const FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void FindHashGridEntitiesInSphere(const FVector Location,const double Radius, TArray<FEntityHandleWrapper>& Entities ,const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static void FindClosestHashGridEntityInSphere(const FVector Location,const double Radius, FEntityHandleWrapper& Entity, const UObject* WorldContextObject,EReturnSuccess& ReturnBranch);
	
	/**
	 * Sets an entity's fragment data or adds it if it isn't present.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject"))
	static void SetEntityFragment(FEntityHandleWrapper Entity, FInstancedStructBPWrapper Fragment,const UObject* WorldContextObject);
	

	UFUNCTION(BlueprintCallable, Category = "Mass", meta=(WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static FInstancedStructBPWrapper GetEntityFragmentByType(FEntityHandleWrapper Entity, FInstancedStructBPWrapper Fragment,const UObject* WorldContextObject, EReturnSuccess& ReturnBranch);

	/*
	* Create a new Struct Instance from the given source data.
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Utilities|Instanced Struct", meta = (CustomStructureParam = "Data", ExpandEnumAsExecs = "ExecResult"))
	static FInstancedStructBPWrapper MakeInstancedStruct(EReturnSuccess& ExecResult, const int32& Data);

	/*
	* Retrieves data from a struct instance if it matches the provided type.
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Utilities|Instanced Struct", meta = (CustomStructureParam = "Data", ExpandEnumAsExecs = "ExecResult"))
	static void BreakInstancedStruct(EReturnSuccess& ExecResult, const FInstancedStructBPWrapper& InstancedStruct, int32& Data);

	/*
	* Resets an InstancedStruct.
	*/
	UFUNCTION(BlueprintCallable, Category = "Utilities|Instanced Struct", meta = (AdvancedDisplay = "1"))
	static void Reset(UPARAM(Ref)FInstancedStructBPWrapper& InstancedStruct, const UScriptStruct* StructType = nullptr)
	{
		InstancedStruct.Struct.InitializeAs(StructType, nullptr);
	}

	/*
	* Checks whether an InstancedStruct is valid or contains any data
	*/
	UFUNCTION(BlueprintCallable, Category = "Utilities|Instanced Struct", meta = (DisplayName = "Is Valid", ExpandEnumAsExecs = "ReturnValue"))
	static EReturnSuccess IsInstancedStructValid(UPARAM(Ref)const FInstancedStructBPWrapper& Fragment)
	{
		return Fragment.Struct.IsValid() ? EReturnSuccess::Success : EReturnSuccess::Failure;
	}

	/*
	* Checks whether an InstancedStruct is empty/null (opposite of IsValid)
	*/
	UFUNCTION(BlueprintCallable, Category = "Utilities|Instanced Struct", meta = (DisplayName = "Is Null"))
	static bool IsInstancedStructNull(UPARAM(Ref)const FInstancedStructBPWrapper& InstancedStruct)
	{
		return InstancedStruct.Struct.GetMemory() == nullptr && InstancedStruct.Struct.GetScriptStruct() == nullptr;
	}

	/*
	* Checks whether two InstancedStructs (and the data contained within) are equal.
	*/
	UFUNCTION(BlueprintPure, Category = "Utilities|Instanced Struct", meta = (CompactNodeTitle = "==", DisplayName = "Equal", Keywords = "== equal"))
	static bool EqualEqual_InstancedStruct(const FInstancedStructBPWrapper& A, const FInstancedStructBPWrapper& B) { return A.Struct == B.Struct; }

	/*
	* Checks whether two InstancedStructs are not equal.
	*/
	UFUNCTION(BlueprintPure, Category = "Utilities|Instanced Struct", meta = (CompactNodeTitle = "!=", DisplayName = "Not Equal", Keywords = "!= not equal"))
	static bool NotEqual_InstancedStruct(const FInstancedStructBPWrapper& A, const FInstancedStructBPWrapper& B) { return A.Struct != B.Struct; }

private:
	DECLARE_FUNCTION(execMakeInstancedStruct);
	DECLARE_FUNCTION(execBreakInstancedStruct);
	
};