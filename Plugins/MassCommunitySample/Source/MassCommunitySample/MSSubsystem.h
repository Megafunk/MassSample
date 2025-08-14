// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassSubsystemBase.h"
#include "NavigationSystem.h"
#include "Common/Fragments/MSOctreeFragments.h"
#include "MSSubsystem.generated.h"

/**
 *  A simple world subsystem that stores our octree and shows some simple mass examples
 *  This inherits from UMassSubsystemBase whichs helps making it easier to use in a Mass Queries
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	FMSOctree2 MassSampleOctree2;
	
	UFUNCTION(BlueprintCallable)
	int32 SampleSpawnEntityExamples();
};

template<>
struct TMassExternalSubsystemTraits<UMSSubsystem>  final
{
	enum
	{
		GameThreadOnly = false,
		ThreadSafeWrite = false,
	};
};


UCLASS(Config = Game, DefaultConfig)
class MASSCOMMUNITYSAMPLE_API UMassSampleSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(config, EditAnywhere, Category = "Visible")
	float OctreeBoundsSize = UE_LARGE_HALF_WORLD_MAX;
};