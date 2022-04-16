// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"

#include "MassEntityTypes.h"

#include "MSBPFunctionLibarary.generated.h"


USTRUCT(BlueprintType)
struct FEntityHandleWrapper
{
	GENERATED_BODY()
	FMassEntityHandle Entity;
	
};


USTRUCT(BlueprintType)
struct FFragmentWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (BaseStruct = "MassFragment", ExcludeBaseStruct))
	FInstancedStruct Fragment;

};

UCLASS()
class MASSSAMPLE_API UMSBPFunctionLibarary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FEntityHandleWrapper SpawnEntityFromEntityConfig(AActor* Owner, UMassEntityConfigAsset* MassEntityConfig, const UObject* WorldContextObject);

	//todo: Lazy fragment-specific versions until we can think of something nicer
	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityTransform(const FEntityHandleWrapper EntityHandle,const FTransform Transform, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetEntityTransform(FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void SetEntityForce(FEntityHandleWrapper EntityHandle, FVector Force, const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void FindHashGridEntitiesInSphere(const FVector Location, double Radius, TArray<FEntityHandleWrapper>& Entities ,const UObject* WorldContextObject);


	UFUNCTION(BlueprintCallable, Category = "Mass", meta = (WorldContext = "WorldContextObject"))
	static void AddFragmentToEntity(FFragmentWrapper Fragment , FEntityHandleWrapper Entity ,const UObject* WorldContextObject);
	
};
