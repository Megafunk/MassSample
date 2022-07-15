// Fill out your copyright notice in the Description page of Project Settings.


#include "LODExampleProcessors.h"

#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "MassSimulationLOD.h"

ULODCollectorExampleProcessor::ULODCollectorExampleProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LODCollector;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorldToMass);
}

ULODExampleProcessor::ULODExampleProcessor()
{
	
}

void ULODExampleProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void ULODExampleProcessor::ConfigureQueries()
{
	EntityQueryBase.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	// Dont perform logic if outside view
	// @note CulledByFrustumTag appears to only be added when the LOD tag is FMassOffLODTag
	EntityQueryBase.AddTagRequirement<FMassVisibilityCulledByFrustumTag>(EMassFragmentPresence::None);

	//Chunk fragments to tick at specified intervals
	EntityQueryBase.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQueryBase.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);

	// Add base queries to high/med/low entity queries
	EntityQuery_High = EntityQueryBase;
	EntityQuery_Medium = EntityQueryBase;
	EntityQuery_Low = EntityQueryBase;
	
	EntityQuery_High.AddTagRequirement<FMassHighLODTag>(EMassFragmentPresence::All); // Query for high LOD
	EntityQuery_Medium.AddTagRequirement<FMassMediumLODTag>(EMassFragmentPresence::All); // Query for medium LOD
	EntityQuery_Low.AddTagRequirement<FMassLowLODTag>(EMassFragmentPresence::All); // Query for low

	//@note it seems like when FMassOffLODTag is set, it will fluctuate between Off and High LOD settings.
	// I can only guess that it is a bug
}

void ULODExampleProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	// High LOD logic
	EntityQuery_High.ForEachEntityChunk(EntitySubsystem, Context, [this](const FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		for (int EntityIdx = 0; EntityIdx < Context.GetNumEntities(); EntityIdx++)
		{
			const FVector& Position = Transforms[EntityIdx].GetTransform().GetLocation();
			DrawDebugPoint(GetWorld(), Position+(FVector::UpVector*500.f), 20.f, FColor::Green, false, 0.25f);
		}
	});

	// Med LOD logic
	EntityQuery_Medium.ForEachEntityChunk(EntitySubsystem, Context, [this](const FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		for (int EntityIdx = 0; EntityIdx < Context.GetNumEntities(); EntityIdx++)
		{
			const FVector& Position = Transforms[EntityIdx].GetTransform().GetLocation();
			DrawDebugPoint(GetWorld(), Position+(FVector::UpVector*500.f), 20.f, FColor::Yellow, false, 0.25f);
		}
	});

	// Low LOD logic
	EntityQuery_Low.ForEachEntityChunk(EntitySubsystem, Context, [this](const FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		for (int EntityIdx = 0; EntityIdx < Context.GetNumEntities(); EntityIdx++)
		{
			const FVector& Position = Transforms[EntityIdx].GetTransform().GetLocation();
			DrawDebugPoint(GetWorld(), Position+(FVector::UpVector*500.f), 20.f, FColor::Red, false, 0.25f);
		}
	});
}
