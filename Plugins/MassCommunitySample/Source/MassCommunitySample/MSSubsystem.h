// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "NavigationSystem.h"
#include "Common/Fragments/MSHashGridFragments.h"
#include "MSSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	TSharedPtr<FMassEntityManager> EntityManager;
	
	FMassArchetypeHandle MoverArchetype;

	//FMSSpatialHash SpatialHashGrid;
	FMSHashGrid3D HashGrid = FMSHashGrid3D(100.0f,FMassEntityHandle());
	
	UPROPERTY()
	UNavigationSystemV1* NavSystem;

	UFUNCTION(BlueprintCallable)
	int32 SpawnEntity();

	virtual void Deinitialize() override
	{
		EntityManager.Reset();
	};
};
