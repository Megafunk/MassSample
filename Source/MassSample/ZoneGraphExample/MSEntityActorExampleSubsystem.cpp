// Fill out your copyright notice in the Description page of Project Settings.


#include "ZoneGraphExample/MSEntityActorExampleSubsystem.h"


#include "MassActorSubsystem.h"
#include "MassAgentComponent.h"
#include "MassAgentSubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MSEntityActorExampleComponent.h"
#include "ZoneGraphSubsystem.h"


void UMSEntityActorExampleSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	
}

void UMSEntityActorExampleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UMassSimulationSubsystem>();

	const UWorld* World = GetWorld();

	ActorSubsystem = World->GetSubsystem<UMassActorSubsystem>();

	SignalSubsystem = Collection.InitializeDependency<UMassSignalSubsystem>();
	checkfSlow(SignalSubsystem != nullptr, TEXT("MassSignalSubsystem is required"));

	AgentSubsystem = Collection.InitializeDependency<UMassAgentSubsystem>();
	checkfSlow(AgentSubsystem != nullptr, TEXT("MassAgentSubsystem is required"));

	AgentSubsystem->GetOnMassAgentComponentEntityAssociated().AddLambda([this](const UMassAgentComponent& AgentComponent)
	{
		if (UMSEntityActorExampleComponent* CapsuleComponent = AgentComponent.GetOwner()->FindComponentByClass<UMSEntityActorExampleComponent>())
		{
			RegisterForComponent(AgentComponent.GetEntityHandle(), *CapsuleComponent);
		}
	});

	AgentSubsystem->GetOnMassAgentComponentEntityDetaching().AddLambda([this](const UMassAgentComponent& AgentComponent)
	{
		if (UMSEntityActorExampleComponent* CapsuleComponent = AgentComponent.GetOwner()->FindComponentByClass<UMSEntityActorExampleComponent>())
		{
			UnregisterForComponent(AgentComponent.GetEntityHandle(), *CapsuleComponent);
		}
	});
	
}


void UMSEntityActorExampleSubsystem::RegisterForComponent(const FMassEntityHandle Entity, UMSEntityActorExampleComponent& TeamTrackingComponent)
{
	EntityToComponentMap.Add(Entity, &TeamTrackingComponent);
	ComponentToEntityMap.Add(&TeamTrackingComponent, Entity);

	AActor* Owner = TeamTrackingComponent.GetOwner();

	const TObjectKey<const AActor> Ownera = Owner;

	ActorSubsystem->SetHandleForActor(Ownera, Entity);
}

void UMSEntityActorExampleSubsystem::UnregisterForComponent(const FMassEntityHandle Entity, UMSEntityActorExampleComponent& TeamTrackingComponent)
{
	EntityToComponentMap.Remove(Entity);
	ComponentToEntityMap.Remove(&TeamTrackingComponent);
}

void UMSEntityActorExampleSubsystem::Deinitialize()
{
	checkfSlow(AgentSubsystem != nullptr, TEXT("No MassAgentSubsystem, MassAgentSubsystem must have be set during initialization"));
	AgentSubsystem->GetOnMassAgentComponentEntityAssociated().RemoveAll(this);
	AgentSubsystem->GetOnMassAgentComponentEntityDetaching().RemoveAll(this);

	Super::Deinitialize();
}
