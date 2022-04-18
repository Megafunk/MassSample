// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSEntityActorExampleSubsystem.generated.h"



class UMassAgentSubsystem;
class UMassEntitySubsystem;
class UMassActorSubsystem;
class UMassSignalSubsystem;
class UCapsuleComponent;



class UMSEntityActorExampleComponent;

/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSEntityActorExampleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()


	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	void RegisterForComponent(const FMassEntityHandle Entity, UMSEntityActorExampleComponent& TeamTrackingComponent);
	void UnregisterForComponent(FMassEntityHandle Entity, UMSEntityActorExampleComponent& TeamTrackingComponent);
	
	
	
	UPROPERTY(Transient)
	UMassEntitySubsystem* EntitySystem;
	
	UPROPERTY(Transient)
	UMassSignalSubsystem* SignalSubsystem;

	UPROPERTY(Transient)
	UMassAgentSubsystem* AgentSubsystem;

	UPROPERTY(Transient)
	UMassActorSubsystem* ActorSubsystem;

	UPROPERTY()
	TMap<UActorComponent*, FMassEntityHandle> ComponentToEntityMap;

	UPROPERTY()
	TMap<FMassEntityHandle, UActorComponent*> EntityToComponentMap;


	
	
};
