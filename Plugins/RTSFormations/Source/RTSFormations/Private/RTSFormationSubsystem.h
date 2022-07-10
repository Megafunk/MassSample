// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Misc/MSBPFunctionLibrary.h"
#include "RTSFormationSubsystem.generated.h"

class UMassAgentComponent;
struct FMassEntityHandle;
/**
 * 
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Stores the num of units in the formation
	UPROPERTY()
	TArray<FMassEntityHandle> Units;

	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);
};
