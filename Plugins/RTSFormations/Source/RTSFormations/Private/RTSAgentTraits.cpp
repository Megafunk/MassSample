// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTraits.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"

void URTSFormationAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FRTSFormationAgent>();
}

URTSFormationInitializer::URTSFormationInitializer()
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSFormationInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);

	MoveEntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	MoveEntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	MoveEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationInitializer::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void URTSFormationInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Error, TEXT("RAN EXECUTE"));
	// First query is to give all units an appropriate unit index.
	// @todo The unit index is given 'randomly' regardless of distance to closest formation position.
	int UnitIndex = 0;
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this, &UnitIndex](FMassExecutionContext& Context)
	{
		TArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			RTSFormationAgent.UnitIndex = UnitIndex;
			UnitIndex++;
		}
	});

	MoveEntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &UnitIndex](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

			const float UnitFloat = UnitIndex;
			const int SideLength = FMath::CeilToInt(FMath::Sqrt(UnitFloat));
			const int BufferDistance = 200.f;
			for(int w = 0;w<SideLength;++w)
			{
				for(int l = 0;l<SideLength;++l)
				{
					// This can be significantly optimized by calculating x/y once rather than looping
					// My small brain just cant comprehend it right now
					int CurrentIndex = w*SideLength+l;
					if (CurrentIndex == RTSFormationAgent.UnitIndex)
					{
						MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());
						MoveTarget.Center = FVector(w*BufferDistance,l*BufferDistance,0.f);
						MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
						MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
						//MoveTarget.DesiredSpeed = FMassInt16Real(200.f);
						UE_LOG(LogTemp, Error, TEXT("MOVE TARGET: %s"), *(MoveTarget.Center.ToString()));
					}
				}
			}
		}
	});
}

void URTSAgentMovement::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSAgentMovement::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();
			
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
		}
	});
}
