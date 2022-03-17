// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "UObject/Object.h"
#include "MSampleSubSystem.generated.h"

/**
 * 
 */
UCLASS()
//TODO: leaving mass out of the name here to avoid confusion. Perhaps change the other classes too?
// FIXME: Find a common prefix for all the filenames of the module. Can do UMassBlah...
class MASSSAMPLE_API UMSampleSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(Transient)
	UMassEntitySubsystem* EntitySystem;
};
