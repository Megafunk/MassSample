// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentSubsystem.h"

#include "LaunchEntityProcessor.h"
#include "MassCommandBuffer.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

void URTSAgentSubsystem::LaunchEntities(const FVector& Location, float Radius) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FindNearbyEntities"));

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	// Query items in radius
	TArray<FMassEntityHandle> Entities;
	const FBox Bounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
	AgentHashGrid.Query(Bounds,Entities);

	if (EntitySubsystem)
	{
		FLaunchEntityFragment LaunchEntityFragment;
		LaunchEntityFragment.Origin = Location;
		
		for(const FMassEntityHandle& Entity : Entities)
		{
			EntitySubsystem->Defer().PushCommand(FCommandAddFragmentInstance(Entity,
				FConstStructView::Make(LaunchEntityFragment)));
		}
	}
}
