// Fill out your copyright notice in the Description page of Project Settings.


#include "ZoneGraphExample/Processors/MSPathFindingProcessor.h"

#include "GraphAStar.h"
#include "MassCommonFragments.h"

#include "ZoneGraphSubsystem.h"

#include "ZoneGraphExample/Fragments/MSZoneGraphExampleFragments.h"


#include "ZoneGraphAStar.h"

//FZoneGraphPathTestFromFragment

UMSPathFindingProcessor::UMSPathFindingProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSPathFindingProcessor::ConfigureQueries()
{

	FromQuery.AddRequirement<FZoneGraphPathTestFromFragment>(EMassFragmentAccess::ReadWrite);
	FromQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);


	ToQuery.AddRequirement<FZoneGraphPathTestToFragment>(EMassFragmentAccess::ReadWrite);
	ToQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSPathFindingProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{



	const UWorld* World = GetWorld();

	FTransform ToTransform;

	ToQuery.ForEachEntityChunk(EntitySubsystem, Context, [World, &ToTransform](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FZoneGraphPathTestToFragment> FromFragmentList = Context.GetMutableFragmentView<FZoneGraphPathTestToFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			ToTransform = TransformList[EntityIndex].GetTransform();
		}
	});
	

	FromQuery.ForEachEntityChunk(EntitySubsystem, Context, [World, this, &ToTransform](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FZoneGraphPathTestFromFragment> FromFragmentList = Context.GetMutableFragmentView<FZoneGraphPathTestFromFragment>();

		UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{

			if (FromFragmentList[EntityIndex].CurrentlyCachedMovement == true)
			{
				// TODO: Make a debug thing here that doesn't just spam
				continue;
			}

			FTransform FromTransform = TransformList[EntityIndex].GetTransform();

			
			// Draw a line between the two points
			DrawLineBetweenTwoPoints(FromTransform, ToTransform);



			// Get ZoneGraph lanes
			// Find the closest ZoneGraph lane to FromTransform
			FZoneGraphLaneLocation FromLane;
			bool bFromLaneSuccess = GetLaneClosestToTransform(FromTransform, FromLane, 250);

			// Find the closest ZoneGraph lane to ToTransform
			FZoneGraphLaneLocation ToLane;
			bool bToLaneSuccess = GetLaneClosestToTransform(ToTransform, ToLane, 250);

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

			TMap<int, int> OutPathLinkedLanes;


			bool bFoundPath = false;
			

			UE_LOG( LogTemp, Warning, TEXT("CurrentlyCachedMovement != true"));

			UE_LOG( LogTemp, Warning, TEXT("-- A*Star Path --"));

			bFoundPath = FindAStarPath(FromLane, ToLane, OutPathLinkedLanes);

			
			// Debug Draw Path
			if (bFoundPath)
			{
				UE_LOG( LogTemp, Warning, TEXT("bFoundPath"));

				

				// Setting cached means that a new path will only be created once navigation has ended
				FromFragmentList[EntityIndex].CurrentlyCachedMovement = true;

				FromFragmentList[EntityIndex].TargetLocation.Reset();
				FromFragmentList[EntityIndex].TargetLocation = ToLane;

				FromFragmentList[EntityIndex].OutPathLinkedLanes = OutPathLinkedLanes;

				FromFragmentList[EntityIndex].CurrentPathIndex = FromLane.LaneHandle.Index;

				DebugShowLane(FromLane.LaneHandle.DataHandle, OutPathLinkedLanes);
			}
			else
			{
				UE_LOG( LogTemp, Warning, TEXT("No path could be found"));
			}
			
			// NOTE: Here is cringe DFTFindPath
			//TMap<int, int> OutPathLinkedLanes2;
			//bool bFoundPath2 = DFTFindPath(FromLane.LaneHandle, ToLane.LaneHandle, OutPathLinkedLanes2);
			
			
		}
	});
	
}

void UMSPathFindingProcessor::DebugShowLane(FZoneGraphDataHandle& DataHandle ,TMap<int, int> OutPathLinkedLanes)
{
	const UWorld* World = GetWorld();
	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();
	
	for (TPair<int, int> LanePair : OutPathLinkedLanes)
	{
		const FZoneGraphStorage* GraphStorage = ZoneGraphSubsystem->GetZoneGraphStorage(DataHandle);

		FZoneLaneLinkData LinkedLane = GraphStorage->LaneLinks[LanePair.Value];

		FZoneData DestLaneZoneData = GraphStorage->GetZoneDataFromLaneIndex(LinkedLane.DestLaneIndex);

		FColor DebugPathColor = FColor::MakeRandomColor();

		FZoneLaneData DestZoneLaneData = GraphStorage->Lanes[LinkedLane.DestLaneIndex];
		// Draw lane line along points
		for (int32 i = DestZoneLaneData.PointsBegin; i < DestZoneLaneData.PointsEnd - 1; i++)
		{
			const FVector& SegStart = GraphStorage->LanePoints[i];
			const FVector& SegEnd = GraphStorage->LanePoints[i + 1];
			DrawLineBetweenTwoPoints(SegStart, SegEnd, DebugPathColor, 5.f, 300.f);
		}

		// Draw text in middle of lane
		int MidPoint = DestZoneLaneData.PointsBegin + (DestZoneLaneData.GetNumPoints() / 2);

		FString LaneDebugString = "Path idx:" + FString::FromInt(LinkedLane.DestLaneIndex);

		int TextStartPoint = FMath::Min(DestZoneLaneData.PointsBegin + 2, DestZoneLaneData.PointsEnd);
		const FVector& TextPosStart = GraphStorage->LanePoints[TextStartPoint];
		DrawDebugString(World, TextPosStart, LaneDebugString, nullptr, DebugPathColor, 300.f, true, 1.5f);

		int TextEndPoint = FMath::Max(DestZoneLaneData.PointsEnd - 2, DestZoneLaneData.PointsBegin);
		const FVector& TextPosEnd = GraphStorage->LanePoints[TextEndPoint];
		DrawDebugString(World, TextPosEnd, LaneDebugString, nullptr, DebugPathColor, 300.f, true, 1.5f);
	}
}


bool UMSPathFindingProcessor::FindAStarPath(FZoneGraphLaneLocation& StartLaneLocationHandle, FZoneGraphLaneLocation& EndLaneLocationHandle, TMap<int, int>& OutPathLinkedLanes)
{
	const UWorld* World = GetWorld();
	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();
	const AZoneGraphData* Data = ZoneGraphSubsystem->GetZoneGraphData(StartLaneLocationHandle.LaneHandle.DataHandle);
	bool bFoundPath = false;
	if (Data)
	{
		const FZoneGraphStorage& ZoneGraphStorage = Data->GetStorage();
		FZoneGraphAStarWrapper Graph(ZoneGraphStorage);
		FZoneGraphAStar Pathfinder(Graph);
		
		FZoneGraphAStarNode StartNode(StartLaneLocationHandle.LaneHandle.Index, StartLaneLocationHandle.Position);
		FZoneGraphAStarNode EndNode(EndLaneLocationHandle.LaneHandle.Index, EndLaneLocationHandle.Position);
		FZoneGraphPathFilter PathFilter(ZoneGraphStorage, StartLaneLocationHandle, EndLaneLocationHandle);

		TArray<FZoneGraphAStarWrapper::FNodeRef> ResultPath;
		EGraphAStarResult Result = Pathfinder.FindPath(StartNode, EndNode, PathFilter, ResultPath);
		if (Result == SearchSuccess)
		{
			bFoundPath = true;
			
			for (int32 i = 0; i < ResultPath.Num() - 1; ++i)
			{
				//UE_LOG(LogTemp, Warning, TEXT("i: %d"), i);
				FZoneGraphAStarWrapper::FNodeRef Node = ResultPath[i];
				const FZoneLaneData& Lane = ZoneGraphStorage.Lanes[Node];
				for (int32 j = Lane.LinksBegin; j < Lane.LinksEnd; j++)
				{
					if (ZoneGraphStorage.LaneLinks[j].DestLaneIndex == ResultPath[i+1] && ZoneGraphStorage.LaneLinks[j].Type != EZoneLaneLinkType::Incoming)// && ZoneGraphStorage.LaneLinks[j].HasFlags()
					{
						OutPathLinkedLanes.Add(Node, j);
						break;
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Idx: %d, LaneID: %d"), i, Node);
			}
			//UE_LOG(LogTemp, Warning, TEXT("LaneLink: %d"), EndNode.NodeRef);
		}
	}
	return bFoundPath;
}

bool UMSPathFindingProcessor::DFTFindPath(FZoneGraphLaneHandle& StartLaneLocationHandle, FZoneGraphLaneHandle& EndLaneLocationHandle, TMap<int, int>& OutPathLinkedLanes)
{
	TSet<int> InProgressLaneIndices;

	UE_LOG( LogTemp, Warning, TEXT("-- DFT Path --"));
	
	const bool bSuccess = DFT_ZoneGraph_Origin(StartLaneLocationHandle, EndLaneLocationHandle, OutPathLinkedLanes, InProgressLaneIndices);
	

	return bSuccess;
}


// TODO: Add bfs algo
// TODO: Add A*star algo

bool UMSPathFindingProcessor::DFT_ZoneGraph_Origin(FZoneGraphLaneHandle& StartLaneLocationHandle, FZoneGraphLaneHandle& EndLaneLocationHandle, TMap<int, int>& OutPathLinkedLanes, TSet<int>& InProgressLaneIndices)
{
	const UWorld* World = GetWorld();
	UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();
	
	bool bFound = false;
	int LaneIndex = StartLaneLocationHandle.Index;

	InProgressLaneIndices.Emplace(LaneIndex);
	
	// NOTE: By using "EZoneLaneLinkType::Outgoing" this ensures respect for lane direction
	// TODO: Add adjacent lane handling (Nah)
	
	const FZoneGraphStorage* Storage = ZoneGraphSubsystem->GetZoneGraphStorage(StartLaneLocationHandle.DataHandle);

	FZoneLaneData LaneData = Storage->Lanes[StartLaneLocationHandle.Index];
	if (LaneData.GetLinkCount() > 0)
	{
		//for (FZoneGraphLinkedLane& LinkedLane : OutLinkedLanes)
		for (int i = LaneData.LinksBegin; i < LaneData.LinksEnd - 1; i++)
		{
			FZoneLaneLinkData LinkData = Storage->LaneLinks[i];
			//UE_LOG( LogTemp, Warning, TEXT("-"));
			FZoneGraphLaneHandle DestLaneHandle = FZoneGraphLaneHandle(LinkData.DestLaneIndex, StartLaneLocationHandle.DataHandle);

			if (!InProgressLaneIndices.Contains(LinkData.DestLaneIndex))
			{
				if (EndLaneLocationHandle.Index == LinkData.DestLaneIndex)//DestLaneHandle.Index
				{
					//UE_LOG( LogTemp, Warning, TEXT("--"));
					bFound = true;
					OutPathLinkedLanes.Add(LaneIndex, i);
					//UE_LOG(LogTemp, Warning, TEXT("LaneLink: %d"), LaneIndex);
					
					//OutPathLinkedLanes.Add(EndLaneLocationHandle.Index, LinkedLane);//FZoneGraphLinkedLane()
					break;
				}
				if (DFT_ZoneGraph_Origin(DestLaneHandle, EndLaneLocationHandle, OutPathLinkedLanes, InProgressLaneIndices) == true)
				{
					//UE_LOG( LogTemp, Warning, TEXT("---"));
					bFound = true;
					OutPathLinkedLanes.Add(LaneIndex, i);
					//UE_LOG(LogTemp, Warning, TEXT("LaneLink: %d"), LaneIndex);
					break;
				}
			}
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT("Failed to get linked lanes"));
	}
	
	return bFound;
}



bool UMSPathFindingProcessor::GetLaneClosestToTransform(FTransform InTransform, FZoneGraphLaneLocation& OutLaneLocation, float QueryMultiplier)
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



void UMSPathFindingProcessor::DrawLineBetweenTwoPoints(FTransform FromTransform, FTransform ToTransform, FColor PathColor, float LineThickness, float LifeTime)
{
	const UWorld* World = GetWorld();
	
	DrawDebugLine(World, FromTransform.GetLocation(), ToTransform.GetLocation(), PathColor, false, LifeTime, 2, /*screenspace*/LineThickness);
}

void UMSPathFindingProcessor::DrawLineBetweenTwoPoints(FVector FromLocation, FVector ToLocation, FColor PathColor, float LineThickness, float LifeTime)
{
	const UWorld* World = GetWorld();
	
	DrawDebugLine(World, FromLocation, ToLocation, PathColor, false, LifeTime, 2, /*screenspace*/LineThickness);
}