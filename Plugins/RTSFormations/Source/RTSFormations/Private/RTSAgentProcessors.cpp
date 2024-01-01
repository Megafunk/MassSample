// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentProcessors.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "RTSAgentTraits.h"
#include "Engine/World.h"

//----------------------------------------------------------------------//
//  URTSUpdateHashPosition
//----------------------------------------------------------------------//
void URTSUpdateHashPosition::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSUpdateHashPosition::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			RTSAgent.CellLoc = AgentSubsystem->AgentHashGrid.Move(Context.GetEntity(EntityIndex), RTSAgent.CellLoc, NewBounds);
		}
	});
}

void URTSUpdateHashPosition::Initialize(UObject& Owner)
{
	AgentSubsystem = UWorld::GetSubsystem<URTSAgentSubsystem>(Owner.GetWorld());
}

//----------------------------------------------------------------------//
//  URTSInitializeHashPosition
//----------------------------------------------------------------------//
URTSInitializeHashPosition::URTSInitializeHashPosition()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSInitializeHashPosition::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSInitializeHashPosition::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			RTSAgent.CellLoc = AgentSubsystem->AgentHashGrid.Add(Context.GetEntity(EntityIndex), NewBounds);
		}
	});
}

void URTSInitializeHashPosition::Initialize(UObject& Owner)
{
	AgentSubsystem = UWorld::GetSubsystem<URTSAgentSubsystem>(Owner.GetWorld());
}

//----------------------------------------------------------------------//
//  URTSRemoveHashPosition
//----------------------------------------------------------------------//

URTSRemoveHashPosition::URTSRemoveHashPosition()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URTSRemoveHashPosition::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
}

void URTSRemoveHashPosition::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> FormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			
			AgentSubsystem->AgentHashGrid.Remove(Context.GetEntity(EntityIndex), RTSAgent.CellLoc);
		}
	});
}

void URTSRemoveHashPosition::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	AgentSubsystem = UWorld::GetSubsystem<URTSAgentSubsystem>(Owner.GetWorld());
}