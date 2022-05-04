// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "StructView.h"

#include "MassEntityTypes.h"

#include "MSBPFunctionLibrary.generated.h"



USTRUCT(BlueprintType)
struct FEntityHandleWrapper
{
	GENERATED_BODY()
	FMassEntityHandle Entity;

};



USTRUCT(BlueprintType)
struct FFragmentBPWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (BaseStruct = "MassFragment", ExcludeBaseStruct))
	FInstancedStruct Struct;

};

USTRUCT(BlueprintType)
struct FStructViewBPWrapper
{
	GENERATED_BODY()


	//not a ustruct, as luck would have it...
	FStructView Struct;

};
UENUM(BlueprintType)
enum EReturnSuccess
{
	Success,
	Failure
};

UCLASS()
class MASSSAMPLE_API UMSBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FEntityHandleWrapper SpawnEntityFromEntityConfig(UMassEntityConfigAsset* MassEntityConfig, const UObject* WorldContextObject, const bool bDebug = false);


	//todo: broken and hardcoded for testing, Don't use this yet!
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FEntityHandleWrapper SpawnEntityFromEntityConfigDeferredTest(AActor* Owner, UMassEntityConfigAsset* MassEntityConfig,
	                                                         const UObject* WorldContextObject);

	//todo: Lazy fragment-specific versions until we can think of something nicer
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityTransform(const FEntityHandleWrapper EntityHandle,const FTransform Transform, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityCollisionQueryIgnoredActors(const FEntityHandleWrapper EntityHandle,const TArray<AActor*> IgnoredActors,const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetEntityTransform(FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityForce(FEntityHandleWrapper EntityHandle, FVector Force, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void FindHashGridEntitiesInSphere(const FVector Location,const double Radius, TArray<FEntityHandleWrapper>& Entities ,const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject",ExpandEnumAsExecs = "ReturnBranch"))
	static void FindClosestHashGridEntityInSphere(const FVector Location,const double Radius, FEntityHandleWrapper& Entity, const UObject* WorldContextObject,TEnumAsByte<EReturnSuccess>& ReturnBranch);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void AddFragmentToEntity(FStructViewBPWrapper Fragment , FEntityHandleWrapper Entity ,const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FString GetEntityDebugString(FEntityHandleWrapper Entity, const UObject* WorldContextObject);







	//thanks to https://forums.unrealengine.com/t/tutorial-how-to-accept-wildcard-structs-in-your-ufunctions/18968/11?u=megafunk


	UFUNCTION(
		BlueprintCallable, 
		Category = "Mass",
		CustomThunk,
		meta = (
			Keywords = "Arbitrary Struct",
			CustomStructureParam = "AnyStruct"
	))
	static void ReceiveSomeStruct(UStruct* AnyStruct);

	DECLARE_FUNCTION(execReceiveSomeStruct)
	{
		// Steps into the stack, walking to the next property in it
		Stack.Step(Stack.Object, NULL);

		// Grab the last property found when we walked the stack
		// This does not contains the property value, only its type information
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

		

		// Grab the base address where the struct actually stores its data
		// This is where the property value is truly stored
		void* StructPtr = Stack.MostRecentPropertyAddress;

		// We need this to wrap up the stack
		P_FINISH;

		ReceiveSomeStruct_impl(StructProperty, StructPtr);
	}

	/*
	* Example function for iterating through all properties of a struct
	* @param StructProperty    The struct property reflection data
	* @param StructPtr        The pointer to the struct value
	*/
	static void ReceiveSomeStruct_impl(FStructProperty* Property, void* StructPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Property->GetClass()->GetName());
		//check for null
		if (Property)
		{


				UE_LOG(LogTemp, Warning, TEXT("%s"), *Property->Struct->GetClass()->GetName());

				
			
		}










			

		
	}
};
