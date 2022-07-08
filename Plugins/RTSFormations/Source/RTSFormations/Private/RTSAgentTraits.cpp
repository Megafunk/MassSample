// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTraits.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"

void URTSFormationAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	check(EntitySubsystem);
	
	BuildContext.AddFragment<FRTSFormationAgent>();
	
	FRTSFormationSettings MyFragment;
	uint32 MySharedFragmentHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(MyFragment));
	FSharedStruct MySharedFragment = EntitySubsystem->GetOrCreateSharedFragment<FRTSFormationSettings>(MySharedFragmentHash, MyFragment);
	BuildContext.AddSharedFragment(MySharedFragment);
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
	MoveEntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationInitializer::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void URTSFormationInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	// First query is to give all units an appropriate unit index.
	// @todo The unit index is given 'randomly' regardless of distance to closest formation position.
	// Ideas: When a unit is already assigned an index, we shouldnt have to recalculate it unless a unit was destroyed and the index destroyed is less than
	// the units index - pretty much like an array, would it be inefficient though?
	int UnitIndex = 0;
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &UnitIndex](FMassExecutionContext& Context)
	{
		TArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			RTSFormationAgent.UnitIndex = UnitIndex;
			UnitIndex++;
		}
	});

	// Square formations have equal sides, so sqrt num units to get side length
	//const float UnitFloat = UnitIndex;
	//const int SideLength = FMath::CeilToInt(FMath::Sqrt(UnitFloat));
			
	//const int BufferDistance = 200.f; // Distance between each unit

	// Query to calculate move target for entities based on unit index
	MoveEntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this, &UnitIndex](FMassExecutionContext& Context)
	{
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const FRTSFormationSettings& RTSFormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();

		const int Whole = RTSFormationSettings.UnitRatioX + RTSFormationSettings.UnitRatioY;
		const int SideLengthX = RTSFormationSettings.UnitRatioX / Whole * UnitIndex;
		const int SideLengthY = RTSFormationSettings.UnitRatioY / Whole * UnitIndex;
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

			// Convert UnitIndex to X/Y coords
			int w = RTSFormationAgent.UnitIndex / SideLengthX;
			int l = RTSFormationAgent.UnitIndex % SideLengthY;

			// We want the formation to be 'centered' so we need to create an offset
			FVector CenterOffset((SideLengthX/2) * RTSFormationSettings.BufferDistance, (SideLengthY/2) * RTSFormationSettings.BufferDistance, 0.f);

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *GetWorld());
			MoveTarget.Center = FVector(w*RTSFormationSettings.BufferDistance-CenterOffset.X,l*RTSFormationSettings.BufferDistance-CenterOffset.Y,0.f);
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 25.f;
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

			// Once we are close enough to our goal, create stand action
			if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
			{
				MoveTarget.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
			}
		}
	});
}
