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
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count,FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override
	{
		
		const FTransform& OwnerTransform = CastChecked<AActor>(&QueryOwner)->GetTransform();

		if (Count <= 0 || !OwnerTransform.IsValid())
		{
			FinishedGeneratingSpawnPointsDelegate.Execute(TArray<FMassEntitySpawnDataGeneratorResult>());
			return;
		}
			
		TArray<FMassEntitySpawnDataGeneratorResult> Results;
			
		for (int32 i = 0; i < EntityTypes.Num(); i++)
		{
			const FMassSpawnedEntityType& EntityType = EntityTypes[i];
			const int32 EntityCount = int32(Count * EntityType.Proportion);

			if (EntityCount > 0)
			{
				FMassEntitySpawnDataGeneratorResult& Res = Results.AddDefaulted_GetRef();
				Res.NumEntities = EntityCount;
				Res.EntityConfigIndex = i;
				Res.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
				
				Res.SpawnData.InitializeAs<FMassTransformsSpawnData>();
				FMassTransformsSpawnData& Transforms = Res.SpawnData.GetMutable<FMassTransformsSpawnData>();
				Transforms.Transforms.Reserve(Res.NumEntities);

				int32 SquareWidth = FMath::Sqrt((float)EntityCount);

				double Spacing = 100.0f;

				
				for (int32 iX = 0; iX < SquareWidth; ++iX)
				{
					for (int32 iY = 0; iY < SquareWidth; ++iY)
					{
						const FVector Point = OwnerTransform.GetLocation() + FVector(Spacing * (iX - SquareWidth), Spacing * (iY - SquareWidth), 0);
						
						Transforms.Transforms.Add(FTransform(OwnerTransform.GetRotation(),Point));
					}
				}
			}
		}
		FinishedGeneratingSpawnPointsDelegate.Execute(Results);
	};
};
