// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphPathing/MSZoneGraphPathTestProcessor.h"

#include "MassCommonFragments.h"
#include "ZoneGraphSubsystem.h"
//#include "Fragments/MSFragments.h"
#include "ZoneGraphExample/Fragments/MSZoneGraphExampleFragments.h"

UMSZoneGraphPathTestProcessor::UMSZoneGraphPathTestProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSZoneGraphPathTestProcessor::ConfigureQueries()
{
	//Super::ConfigureQueries();

	FromQuery.AddRequirement<FZoneGraphPathTestFromFragment>(EMassFragmentAccess::ReadWrite);
	FromQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);


	ToQuery.AddRequirement<FZoneGraphPathTestToFragment>(EMassFragmentAccess::ReadWrite);
	ToQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSZoneGraphPathTestProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	//Super::Execute(EntitySubsystem, Context);

	return;

	const UWorld* World = GetWorld();

	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();

	FTransform FromTransform;

	FromQuery.ForEachEntityChunk(EntitySubsystem, Context, [World, &FromTransform](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FZoneGraphPathTestFromFragment> FromFragmentList = Context.GetMutableFragmentView<FZoneGraphPathTestFromFragment>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			//FromTransform = TransformList[EntityIndex].GetTransform();

			FromTransform = FromFragmentList[EntityIndex].FromTransform;
		}
	});


	FTransform ToTransform;

	ToQuery.ForEachEntityChunk(EntitySubsystem, Context, [World, &ToTransform](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FZoneGraphPathTestToFragment> FromFragmentList = Context.GetMutableFragmentView<FZoneGraphPathTestToFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			//ToTransform = TransformList[EntityIndex].GetTransform();
			
			ToTransform = FromFragmentList[EntityIndex].ToTransform;
		}
	});


	// Draw a line between the two points
	DrawLineBetweenTwoPoints(FromTransform, ToTransform);



	// Get ZoneGraph lanes
	// Find the closest ZoneGraph lane to FromTransform
	FZoneGraphLaneLocation FromLane;
	bool bFromLaneSuccess = GetLaneClosestToTransform(FromTransform, FromLane);

	// Find the closest ZoneGraph lane to ToTransform
	FZoneGraphLaneLocation ToLane;
	bool bToLaneSuccess = GetLaneClosestToTransform(ToTransform, ToLane);

	// Draw lines from points to lanes
	if (bFromLaneSuccess)
	{
		DrawLineBetweenTwoPoints(FromTransform.GetLocation(), FromLane.Position, FColor::Blue);
	}
	if (bToLaneSuccess)
	{
		DrawLineBetweenTwoPoints(ToTransform.GetLocation(), ToLane.Position, FColor::Blue);
	}

	// PathFinding

	TArray<FZoneGraphLinkedLane> OutPathLinkedLanes;

	bool bFoundPath = DFTFindPath(FromLane.LaneHandle, ToLane.LaneHandle,OutPathLinkedLanes);


	// Debug Draw Path
	if (bFoundPath)
	{
		for (FZoneGraphLinkedLane& LinkedLane : OutPathLinkedLanes)
		{
			const FZoneGraphStorage* GraphStorage = ZoneGraphSubsystem->GetZoneGraphStorage(LinkedLane.DestLane.DataHandle);
			
			FZoneData DestLaneZoneData = GraphStorage->GetZoneDataFromLaneIndex(LinkedLane.DestLane.Index);
			
			FZoneLaneData DestZoneLaneData = GraphStorage->Lanes[LinkedLane.DestLane.Index];
			for (int32 i = DestZoneLaneData.PointsBegin; i < DestZoneLaneData.PointsEnd - 1; i++)
			{
				const FVector& SegStart = GraphStorage->LanePoints[i];
				const FVector& SegEnd = GraphStorage->LanePoints[i + 1];
				DrawLineBetweenTwoPoints(SegStart, SegEnd, FColor::Cyan);
			}
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT("No path could be found"));
	}
	
}

bool UMSZoneGraphPathTestProcessor::DFTFindPath(FZoneGraphLaneHandle& StartLaneLocationHandle, FZoneGraphLaneHandle& EndLaneLocationHandle, TArray<FZoneGraphLinkedLane>& OutPathLinkedLanes)
{
	TSet<int> InProgressLaneIndices;
	
	const bool bSuccess = DFT_ZoneGraph_Origin(StartLaneLocationHandle, EndLaneLocationHandle, OutPathLinkedLanes, InProgressLaneIndices);
	

	return bSuccess;
}



bool UMSZoneGraphPathTestProcessor::DFT_ZoneGraph_Origin(FZoneGraphLaneHandle& StartLaneLocationHandle, FZoneGraphLaneHandle& EndLaneLocationHandle, TArray<FZoneGraphLinkedLane>& OutPathLinkedLanes, TSet<int>& InProgressLaneIndices)
{
	const UWorld* World = GetWorld();
	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();
	
	bool bFound = false;
	int LaneIndex = StartLaneLocationHandle.Index;

	InProgressLaneIndices.Emplace(LaneIndex);
	
	// NOTE: By using "EZoneLaneLinkType::Outgoing" this ensures respect for lane direction
	TArray<FZoneGraphLinkedLane> OutLinkedLanes;
	bool bLinkedSuccess = ZoneGraphSubsystem->GetLinkedLanes(StartLaneLocationHandle, EZoneLaneLinkType::Outgoing, EZoneLaneLinkFlags::All, EZoneLaneLinkFlags::None, OutLinkedLanes);
	
	if (bLinkedSuccess)
	{
		for (FZoneGraphLinkedLane& LinkedLane : OutLinkedLanes)
		{
			UE_LOG( LogTemp, Warning, TEXT("-"));
			FZoneGraphLaneHandle DestLaneHandle = LinkedLane.DestLane;

			if (!InProgressLaneIndices.Contains(DestLaneHandle.Index))
			{
				if (EndLaneLocationHandle.Index == DestLaneHandle.Index)
				{
					UE_LOG( LogTemp, Warning, TEXT("--"));
					bFound = true;
					OutPathLinkedLanes.Add(LinkedLane);
					break;
				}
				if (DFT_ZoneGraph_Origin(DestLaneHandle, EndLaneLocationHandle, OutPathLinkedLanes, InProgressLaneIndices) == true)
				{
					UE_LOG( LogTemp, Warning, TEXT("---"));
					bFound = true;
					OutPathLinkedLanes.Add(LinkedLane);
					break;
				}
			}
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT("Filed to get linked lanes"));
	}
	
	return bFound;
}



bool UMSZoneGraphPathTestProcessor::GetLaneClosestToTransform(FTransform InTransform, FZoneGraphLaneLocation& OutLaneLocation, float QueryMultiplier)
{
	const UWorld* World = GetWorld();
	
	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();

	if (!ZoneGraphSubsystem)
	{
		UE_LOG( LogTemp, Warning, TEXT("UZoneGraphSubsystem does not exist"));
		return false;
	}

	const FBox QueryBox = FBox(InTransform.GetLocation() + FVector(-1 * QueryMultiplier,-1 * QueryMultiplier,-1 * QueryMultiplier),InTransform.GetLocation() + FVector(1 * QueryMultiplier,1 * QueryMultiplier,1 * QueryMultiplier));

	//ZoneGraphTagFilter

	float OutDistanceSqr = .0f;
	
	FZoneGraphTagFilter NoTags;

	bool bSuccess = ZoneGraphSubsystem->FindNearestLane(QueryBox, NoTags,OutLaneLocation, OutDistanceSqr);

	return bSuccess;
}



void UMSZoneGraphPathTestProcessor::DrawLineBetweenTwoPoints(FTransform FromTransform, FTransform ToTransform, FColor PathColor, float LineThickness)
{
	const UWorld* World = GetWorld();
	
	DrawDebugLine(World, FromTransform.GetLocation(), ToTransform.GetLocation(), PathColor, false, 2.0f, 2, /*screenspace*/LineThickness);
}

void UMSZoneGraphPathTestProcessor::DrawLineBetweenTwoPoints(FVector FromLocation, FVector ToLocation, FColor PathColor, float LineThickness)
{
	const UWorld* World = GetWorld();
	
	DrawDebugLine(World, FromLocation, ToLocation, PathColor, false, 2.0f, 2, /*screenspace*/LineThickness);
}