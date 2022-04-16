// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "Fragments/MSHashGridFragments.h"
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

	FMSHashGrid3D HashGrid = FMSHashGrid3D(100.0f,FMassEntityHandle());

	UFUNCTION(BlueprintCallable)
	int32 SpawnEntity();
};
