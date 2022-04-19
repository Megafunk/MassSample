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
 * This subsytem demonstates 2 forms of actor/component(object) tacking and storage.
 * The 1st is the setting of EntityHandles for the owners of any actors that have a "UMSEntityActorExampleComponent" component. This applies for any actor with a MassAgent Component,
 * and the process of setting the handles is added via the "AgentSubsystem->GetOnMassAgentComponentEntityAssociated()" lambda.
 * This is just an example it nice setup here: "UMSBPFunctionLibarary".
 * The 2nd is the use of a TMap to setup direct maps from component to entity within this subsystem, the advantage of this im assuming is that it keeps the referencing and functionality,
 * that the subsystem may be intended for within this place, anytime I might want to run Mass stuff relating to 'Gorbo' the 'GorboComponents' and other stuff is gonna be here.
 */
UCLASS()
class MASSSAMPLE_API UMSEntityActorExampleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()


	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	void RegisterForComponent(const FMassEntityHandle Entity, UMSEntityActorExampleComponent& EntityActorExampleComponent);
	void UnregisterForComponent(FMassEntityHandle Entity, UMSEntityActorExampleComponent& EntityActorExampleComponent);
	
	
	
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
