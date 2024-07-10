// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentSubsystem.h"

#include "LaunchEntityProcessor.h"
#include "MassCommandBuffer.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

void URTSAgentSubsystem::LaunchEntities(const FVector& Location, float Radius) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("LaunchEntities"));

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	// Query items in radius
	TArray<FMassEntityHandle> Entities;
	const FBox Bounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
	AgentHashGrid.QuerySmall(Bounds,Entities);

	if (EntitySubsystem)
	{
		FLaunchEntityFragment LaunchEntityFragment;
		LaunchEntityFragment.Origin = Location;
		LaunchEntityFragment.Magnitude = 500.f;

		
		for(const FMassEntityHandle& Entity : Entities)
		{
			EntitySubsystem->Defer().PushCommand(FCommandAddFragmentInstance(Entity,
				FConstStructView::Make(LaunchEntityFragment)));
		}
		// hacky fix since I couldnt get observer working for the life of me
		if (Entities.Num())
			GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntities(LaunchEntity, Entities,0.1f);
	}
}
