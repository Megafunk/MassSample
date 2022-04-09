// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "UObject/Object.h"
#include "MSSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(Transient)
	UMassEntitySubsystem* EntitySystem;
	FMassArchetypeHandle MoverArchetype;


	UFUNCTION(BlueprintCallable)
	int32 SpawnEntity();
};
