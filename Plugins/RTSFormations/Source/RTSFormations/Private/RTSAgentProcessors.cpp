// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentProcessors.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
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
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSUpdateHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			RTSAgent.CellLoc = AgentSubsystem.AgentHashGrid.Move(Context.GetEntity(EntityIndex), RTSAgent.CellLoc, NewBounds);
		}
	});
}

void URTSUpdateHashPosition::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
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
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::None);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSInitializeHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			UE_LOG(LogTemp, Log, TEXT("Agents: %d"), AgentSubsystem.AgentHashGrid.GetItems().Num());
			RTSAgent.CellLoc = AgentSubsystem.AgentHashGrid.Add(Context.GetEntity(EntityIndex), NewBounds);
			
			Context.Defer().AddTag<FRTSAgentHashTag>(Context.GetEntity(EntityIndex));
		}
	});
}

void URTSInitializeHashPosition::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
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
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSRemoveHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> FormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& RTSAgent = FormationAgents[EntityIndex];
			
			AgentSubsystem.AgentHashGrid.Remove(Context.GetEntity(EntityIndex), RTSAgent.CellLoc);
		}
	});
}

void URTSRemoveHashPosition::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}