// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassObserverRegistry.h"
#include "Common/Misc/MSBPFunctionLibrary.h"
#include "UObject/Object.h"
#include "MSObserverProcessor.generated.h"

UENUM(BlueprintType)
enum class EMassObservedOperationBP : uint8
{
	Add,
	Remove,
};

/**
 * 
 */
UCLASS(Blueprintable, Abstract, meta=(ShowWorldContextPin))
class MASSCOMMUNITYSAMPLE_API UMSObserverProcessorBP : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSObserverProcessorBP();

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	UFUNCTION(BlueprintImplementableEvent, meta=(WorldContext="WorldContextObject"))
	void BPExecute(FMSEntityViewBPWrapper EntityHandle, const UObject* WorldContextObject);


	FMassEntityQuery EntityQuery;


	UPROPERTY(Category="Query", EditAnywhere,
		meta = (BaseStruct = "MassFragment", ExcludeBaseStruct, NoElementDuplicate))
	TArray<FInstancedStruct> FragmentRequirements;
	UPROPERTY(Category="Query", EditAnywhere, meta = (BaseStruct = "/Script/MassEntity.MassTag", ExcludeBaseStruct, NoElementDuplicate))
	TArray<FInstancedStruct> TagRequirements;

	UPROPERTY(Category="Query", EditAnywhere, meta = (BaseStruct = "/Script/MassEntity.MassFragment", ExcludeBaseStruct))
	FInstancedStruct ObservedFragment;

	UPROPERTY(Category="Query", EditAnywhere)
	EMassObservedOperationBP ObservedOperation = EMassObservedOperationBP::Add;

	
};


UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blueprint Observers"))
class MASSCOMMUNITYSAMPLE_API UMSObserverSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(config, EditAnywhere, Category = "Visible", meta = (AllowAbstract = "false"))
	TArray<TSubclassOf<UMSObserverProcessorBP>> ObserversToRegister;
};


UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSObserverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Collection.InitializeDependency<UMassEntitySubsystem>();

		const auto Settings = GetDefault<UMSObserverSettings>();
		

		for (auto ObserverClass : Settings->ObserversToRegister)
		{
			if (!ObserverClass) continue;

			UMSObserverProcessorBP* CDO = ObserverClass.GetDefaultObject();
			
			if (!GetClass()->HasAnyFlags(RF_ClassDefaultObject) && CDO->ObservedFragment.IsValid())

				//TODO @megafunk
				// This CDO (UMassObserverRegistry) remains in memory between PIE sessions... Is there a better way?
				UMassObserverRegistry::GetMutable().RegisterObserver(*CDO->ObservedFragment.GetScriptStruct(),
				                                                     (EMassObservedOperation)CDO->ObservedOperation, ObserverClass);
		}
	};
};
