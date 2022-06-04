// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassObserverRegistry.h"
#include "Common/Misc/MSBPFunctionLibrary.h"
#include "UObject/Object.h"
#include "MSObserverProcessor.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, Abstract, meta=(ShowWorldContextPin))
class MASSSAMPLE_API UMSObserverProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSObserverProcessor();
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	UFUNCTION(BlueprintImplementableEvent, meta=(WorldContext="WorldContextObject"))
	void BPExecute(FEntityHandleWrapper EntityHandle, const UObject* WorldContextObject);


	FMassEntityQuery EntityQuery;


	UPROPERTY(Category="Query", EditAnywhere, meta = (BaseStruct = "MassFragment", ExcludeBaseStruct))
	TArray<FInstancedStruct> FragmentRequirements;
	UPROPERTY(Category="Query", EditAnywhere,  meta = (BaseStruct = "MassTag", ExcludeBaseStruct))
	TArray<FInstancedStruct> TagRequirements;

	UPROPERTY(Category="Query", EditAnywhere, meta = (BaseStruct = "MassFragment", ExcludeBaseStruct))
	FInstancedStruct ObservedFragment;
};


UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blueprint Observers"))
class MASSSAMPLE_API UMSObserverSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	
	UPROPERTY(config, EditAnywhere, Category = "Visible", meta = (AllowAbstract = "false"))
	TArray<TSubclassOf<UMSObserverProcessor>> ObserversToRegister;

};



UCLASS()
class MASSSAMPLE_API UMSObserverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Collection.InitializeDependency<UMassEntitySubsystem>();

		const auto Settings = GetDefault<UMSObserverSettings>();

		for (auto ObserverClass : Settings->ObserversToRegister)
		{
			if(!ObserverClass) continue;
			
			auto CDO = ObserverClass.GetDefaultObject();
			
			if(!GetClass()->HasAnyFlags(RF_ClassDefaultObject) && CDO->ObservedFragment.IsValid())
				
				UMassObserverRegistry::GetMutable().RegisterObserver(*CDO->ObservedFragment.GetScriptStruct(), EMassObservedOperation::Add, ObserverClass);
		}

		

	};
};
