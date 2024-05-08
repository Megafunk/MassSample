// Fill out your copyright notice in the Description page of Project Settings.

#include "MSOctreeProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Chaos/DebugDrawQueue.h"
#include "Common/Fragments/MSOctreeFragments.h"

static TAutoConsoleVariable<bool> CVarMSDrawOctree(
	TEXT("masssample.debugoctree"), false,
	TEXT(
		"Enables debug drawing for the Mass Sample octree example. Uses Chaos debug draw so it can called from other threads so remember to call p.Chaos.DebugDraw.Enabled 1"));

UMSOctreeProcessor::UMSOctreeProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UMSOctreeProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	MassSampleSystem = GetWorld()->GetSubsystem<UMSSubsystem>();
#if CHAOS_DEBUG_DRAW
	Chaos::FDebugDrawQueue::GetInstance().SetConsumerActive(this, true);
#endif
}

void UMSOctreeProcessor::ConfigureQueries()
{
	// Ideally we only do this for meshes that actually moved

	UpdateOctreeElementsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateOctreeElementsQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	UpdateOctreeElementsQuery.AddTagRequirement<FMSInOctreeGridTag>(EMassFragmentPresence::All);
	UpdateOctreeElementsQuery.AddConstSharedRequirement<FMSSharedBaseBounds>(EMassFragmentPresence::Optional);
	UpdateOctreeElementsQuery.RegisterWithProcessor(*this);
}

void UMSOctreeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{


	UpdateOctreeElementsQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UMSOctreeUpdate)
		FMSOctree2& Octree = MassSampleSystem->Octree2;

		const int32 NumEntities = Context.GetNumEntities();

		const auto LocationList = Context.GetFragmentView<FTransformFragment>();
		const auto OctreeFragments = Context.GetMutableFragmentView<FMSOctreeFragment>();
		const auto SharedBaseBounds = Context.GetConstSharedFragmentPtr<FMSSharedBaseBounds>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const auto& Transform = LocationList[i].GetTransform();
			auto OctreeFragment = OctreeFragments[i];

			if (!OctreeFragment.OctreeID)
			{
				continue;
			}


			if (FOctreeElementId2* OctreeID = OctreeFragment.OctreeID.Get(); Octree.IsValidElementId(*OctreeID))
			{
				FMSEntityOctreeElement TempOctreeElement = Octree.GetElementById(*OctreeID);

				Octree.RemoveElement(*OctreeID);

				// expects a 0 W component


				if (SharedBaseBounds)
				{
					TempOctreeElement.Bounds = FBoxCenterAndExtent(SharedBaseBounds->BoxSphereBounds.TransformBy(Transform));
				}
				else
				{
					FVector Location = Transform.GetLocation();
					TempOctreeElement.Bounds.Center = FVector4(Location, 0);
					//DrawDebugBox(EntityManager.GetWorld(), TempOctreeElement.Bounds.Center, TempOctreeElement.Bounds.Extent, FColor::Green);
				}


				Octree.AddElement(TempOctreeElement);
			}
		}
	});
	FMSOctree2& Octree = MassSampleSystem->Octree2;

#if CHAOS_DEBUG_DRAW
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UMSOctreeDebugDraw)
	if (CVarMSDrawOctree.GetValueOnAnyThread())
	{
		Octree.FindAllElements([&](const FMSEntityOctreeElement& Element)
		{
			// p.Chaos.DebugDraw.Enabled and masssample.debugoctree must be true for this to show
			// using the chaos debug draw as it can be done from other threads
			Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Element.Bounds.Center, Element.Bounds.Extent, FQuat::Identity, FColor::White, false, 0,
			                                                   0, 1.0f);
			Chaos::FDebugDrawQueue::GetInstance().DrawDebugString(Element.Bounds.Center + FVector(0, 0, (Element.Bounds.Extent.Z * 2)),
			                                                      FString::FromInt(Element.EntityHandle.Index), nullptr,
			                                                      FColor::MakeRandomSeededColor(Element.EntityHandle.Index), 0, true, 2);
		});
	}
#endif
}

void UMSOctreeProcessor::BeginDestroy()
{
#if CHAOS_DEBUG_DRAW
	Chaos::FDebugDrawQueue::GetInstance().SetConsumerActive(this, false);
#endif
	Super::BeginDestroy();
}

UMSHashGridMemberInitializationProcessor::UMSHashGridMemberInitializationProcessor() : EntityQuery(*this)
{
	ObservedType = FMSOctreeFragment::StaticStruct();
	bRequiresGameThreadExecution = true;
	Operation = EMassObservedOperation::Add;
}

void UMSHashGridMemberInitializationProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	MassSampleSystem = GetWorld()->GetSubsystem<UMSSubsystem>();
}

void UMSHashGridMemberInitializationProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);

	EntityQuery.AddConstSharedRequirement<FMSSharedBaseBounds>(EMassFragmentPresence::Optional);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMSHashGridMemberInitializationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MSHashGridMemberInitialization)
		const auto TransformList = Context.GetFragmentView<FTransformFragment>();
		const auto OctreeList = Context.GetMutableFragmentView<FMSOctreeFragment>();
		const auto RadiusList = Context.GetMutableFragmentView<FAgentRadiusFragment>();
		const auto SharedBaseBounds = Context.GetConstSharedFragmentPtr<FMSSharedBaseBounds>();

		const int32 NumEntities = Context.GetNumEntities();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const auto& Transform = TransformList[i].GetTransform();

			FMSOctree2& Octree = MassSampleSystem->Octree2;


			auto Entity = Context.GetEntity(i);


			FMSEntityOctreeElement NewOctreeElement;


			if (SharedBaseBounds)
			{
				NewOctreeElement.Bounds = FBoxCenterAndExtent(SharedBaseBounds->BoxSphereBounds.TransformBy(Transform));
			}
			else
			{
				if (RadiusList.Num() > 0)
				{
					NewOctreeElement.Bounds = FBoxCenterAndExtent(Transform.GetLocation(), FVector(RadiusList[i].Radius * 2.0f));
				}
				NewOctreeElement.Bounds = FBoxCenterAndExtent(Transform.GetLocation(), FVector(50.0f));
			}

			NewOctreeElement.SharedOctreeID = MakeShared<FOctreeElementId2, ESPMode::ThreadSafe>();
			NewOctreeElement.EntityHandle = Entity;

			Octree.AddElement(NewOctreeElement);

			OctreeList[i].OctreeID = NewOctreeElement.SharedOctreeID;
			Context.Defer().AddTag<FMSInOctreeGridTag>(Context.GetEntity(i));
		}
	});
}

UMSOctreeMemberCleanupProcessor::UMSOctreeMemberCleanupProcessor()
{
	ObservedType = FMSOctreeFragment::StaticStruct();
	bRequiresGameThreadExecution = true;
	Operation = EMassObservedOperation::Remove;
}

void UMSOctreeMemberCleanupProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	MassSampleSystem = GetWorld()->GetSubsystem<UMSSubsystem>();
}

void UMSOctreeMemberCleanupProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMSOctreeMemberCleanupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	auto& Octree = MassSampleSystem->Octree2;

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
	{
		const auto NavigationObstacleCellLocationList = Context.GetMutableFragmentView<FMSOctreeFragment>();

		const int32 NumEntities = Context.GetNumEntities();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (auto OctreeID = NavigationObstacleCellLocationList[i].OctreeID)
			{
				Octree.RemoveElement(*OctreeID);
			}
		}
	});
}
