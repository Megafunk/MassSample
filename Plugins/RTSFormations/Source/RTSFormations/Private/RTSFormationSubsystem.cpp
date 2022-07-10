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
	
	// My current observer implementation doesnt handle entity destruction properly, so the logic is performed here for the time
	Units.Remove(Entity->GetEntityHandle());
	GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(FormationUpdated, Units);
}
