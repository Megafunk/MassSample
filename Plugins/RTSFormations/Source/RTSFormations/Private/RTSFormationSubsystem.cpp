// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "MassAgentComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSignalSubsystem.h"
#include "RTSFormationProcessors.h"

void URTSFormationSubsystem::DestroyEntity(UMassAgentComponent* Entity)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	EntitySubsystem->Defer().DestroyEntity(Entity->GetEntityHandle());
	UE_LOG(LogTemp, Error, TEXT("Entity: %d"), Entity->GetEntityHandle().Index)
	Units.Remove(Entity->GetEntityHandle());
	GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(FormationUpdated, Units);
}
