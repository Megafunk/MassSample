// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassSpawnLocationProcessor.h"
#include "MassSpawnerTypes.h"
#include "SpacedGridLocationsSpawnDataGenerator.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MASSCOMMUNITYSAMPLE_API USpacedGridLocationsSpawnDataGenerator : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	double Spacing = 100.0f;

	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count,
	                      FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override
	{
		const FTransform& OwnerTransform = CastChecked<AActor>(&QueryOwner)->GetTransform();

		if (Count <= 0 || !OwnerTransform.IsValid())
		{
			FinishedGeneratingSpawnPointsDelegate.Execute(TArray<FMassEntitySpawnDataGeneratorResult>());
		}
		TArray<FMassEntitySpawnDataGeneratorResult> Results;
		BuildResultsFromEntityTypes(Count, EntityTypes, Results);


		TArray<int32> NumLeftPerResults;

		
		{
			int32 CountPostBuildResults = 0;
			for (FMassEntitySpawnDataGeneratorResult& Res : Results)
			{
				Res.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
				Res.SpawnData.InitializeAs<FMassTransformsSpawnData>();
				FMassTransformsSpawnData& Transforms = Res.SpawnData.GetMutable<FMassTransformsSpawnData>();
				Transforms.Transforms.SetNumUninitialized(Res.NumEntities);
				NumLeftPerResults.Add(Res.NumEntities);
				CountPostBuildResults += Res.NumEntities;
			}
			Count = CountPostBuildResults;
		}
		

		
		int32 SquaredRows = FMath::Sqrt((float)Count);
		
		const FVector BaseLocation = OwnerTransform.GetLocation();



		// Distribute points in the grid
		TArray<int32> Grid;
		Grid.SetNum(SquaredRows);
		for (int32 i = 0; i < SquaredRows; i++)
		{
			Grid[i] = SquaredRows;
		}
		// Handle leftover points
		int32 Remaining = Count - (SquaredRows * SquaredRows);
		while (Remaining > 0)
		{
			if (Remaining >= SquaredRows)
			{
				// If there's enough points to fill a row, add a row
				int32 NewRow = SquaredRows;
				Grid.Add(NewRow);
				Remaining -= SquaredRows;
			}
			else
			{
				// If there's less than a row's worth of points, add them to a new row
				int32 NewRow = Remaining;
				Grid.Add(NewRow);
				Remaining = 0;
			}
		}


		// Stuff them all into our results randomly
		int32 ResultsIndex = 0;
		for (int32 i = 0; i < Grid.Num(); i++)
		{
			for (int32 j = 0; j < Grid[i]; ++j)
			{
				FTransform NewTransform = FTransform(BaseLocation + FVector(i*Spacing,j*Spacing,0));
				NewTransform.SetRotation(OwnerTransform.Rotator().Quaternion());
				// scatter the transforms we make over the results instead of making more arrays
				while(ResultsIndex <= NumLeftPerResults.Num())
				{
					int32 ResultNumLeft = NumLeftPerResults[ResultsIndex];
					if(ResultNumLeft > 0)
					{
						FMassEntitySpawnDataGeneratorResult& Res = Results[ResultsIndex];
						FMassTransformsSpawnData& TransformData = Res.SpawnData.GetMutable<FMassTransformsSpawnData>();
						TransformData.Transforms[ResultNumLeft - 1] = NewTransform;
						--NumLeftPerResults[ResultsIndex];
						break;
					}
					ResultsIndex++;
				}
			}
			
		}
		
		FinishedGeneratingSpawnPointsDelegate.Execute(Results);
	};
};
