// Fill out your copyright notice in the Description page of Project Settings.

#include "MSOctreeProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Chaos/DebugDrawQueue.h"
#include "Common/Fragments/MSOctreeFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSOctreeProcessors)

static TAutoConsoleVariable<bool> CVarMSDrawOctree(
	TEXT("masssample.debugoctree"), false,
	TEXT(
		"Enables debug drawing for the Mass Sample octree example. Uses Chaos debug draw so it can called from other threads"));

UMSOctreeProcessor::UMSOctreeProcessor() : UpdateOctreeElementsQuery(*this) {
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
}

void UMSOctreeProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager)
{
	Super::InitializeInternal(Owner, Manager);
#if CHAOS_DEBUG_DRAW
	Chaos::FDebugDrawQueue::GetInstance().SetConsumerActive(this, true);
#endif
}

void UMSOctreeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	UpdateOctreeElementsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateOctreeElementsQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	UpdateOctreeElementsQuery.AddTagRequirement<FMSInOctreeGridTag>(EMassFragmentPresence::All);
	UpdateOctreeElementsQuery.AddConstSharedRequirement<FMSSharedBaseBounds>(EMassFragmentPresence::Optional);

	UpdateOctreeElementsQuery.AddSubsystemRequirement<UMSSubsystem>(EMassFragmentAccess::ReadWrite);
	
	UpdateOctreeElementsQuery.RegisterWithProcessor(*this);
}

void UMSOctreeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{

	UpdateOctreeElementsQuery.ForEachEntityChunk( InContext, [this](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UMSOctreeUpdate)

		FMSOctree2& Octree = Context.GetMutableSubsystemChecked<UMSSubsystem>().MassSampleOctree2;

		const int32 NumEntities = Context.GetNumEntities();

		const auto LocationList = Context.GetFragmentView<FTransformFragment>();
		const auto OctreeFragments = Context.GetMutableFragmentView<FMSOctreeFragment>();
		const auto SharedBaseBounds = Context.GetConstSharedFragmentPtr<FMSSharedBaseBounds>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const auto& Transform = LocationList[i].GetTransform();
			// We do not actually need to write this value itself, as it's just a pointer to the octree ID (a bit sad indirection-wise)
			const FMSOctreeFragment& OctreeFragment = OctreeFragments[i];

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

#if CHAOS_DEBUG_DRAW
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UMSOctreeDebugDraw)
	// I would prefer to use the subsystem access from the context but I can't seem to mimick the way the signal subsystem does so
	const UMSSubsystem* MSSubSystem  = nullptr;
	if (auto World = GetWorld()) {
		MSSubSystem = World->GetSubsystem<UMSSubsystem>();
	}

	if (MSSubSystem && CVarMSDrawOctree.GetValueOnAnyThread())
	{
		MSSubSystem->MassSampleOctree2.FindAllElements([&](const FMSEntityOctreeElement& Element)
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
	ObservedOperations = EMassObservedOperationFlags::Add;

	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
}

void UMSHashGridMemberInitializationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);

	EntityQuery.AddConstSharedRequirement<FMSSharedBaseBounds>(EMassFragmentPresence::Optional);

	EntityQuery.AddSubsystemRequirement<UMSSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSHashGridMemberInitializationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{
	EntityQuery.ForEachEntityChunk(InContext, [this](FMassExecutionContext& Context)
	{
		FMSOctree2& Octree = Context.GetMutableSubsystemChecked<UMSSubsystem>().MassSampleOctree2;

		QUICK_SCOPE_CYCLE_COUNTER(STAT_MSHashGridMemberInitialization)
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FMSOctreeFragment> OctreeList = Context.GetMutableFragmentView<FMSOctreeFragment>();
		TConstArrayView<FAgentRadiusFragment> RadiusList = Context.GetFragmentView<FAgentRadiusFragment>();

		// Using a ptr here as it is optional
		const FMSSharedBaseBounds* SharedBaseBounds = Context.GetConstSharedFragmentPtr<FMSSharedBaseBounds>();

		
		const int32 NumEntities = Context.GetNumEntities();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FTransform& Transform = TransformList[i].GetTransform();

			FMassEntityHandle Entity = Context.GetEntity(i);
			
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

UMSOctreeMemberCleanupProcessor::UMSOctreeMemberCleanupProcessor(): EntityQuery(*this) {
	ObservedType = FMSOctreeFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;

	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
}


void UMSOctreeMemberCleanupProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMSOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMSSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSOctreeMemberCleanupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{

	EntityQuery.ForEachEntityChunk( InContext, [this](FMassExecutionContext& Context)
	{
		FMSOctree2& Octree = Context.GetMutableSubsystemChecked<UMSSubsystem>().MassSampleOctree2;

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
