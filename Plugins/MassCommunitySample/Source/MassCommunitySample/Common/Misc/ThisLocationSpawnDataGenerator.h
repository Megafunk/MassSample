// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassSpawnLocationProcessor.h"
#include "MassSpawnerTypes.h"
#include "MassEntityConfigAsset.h"
#include "ThisLocationSpawnDataGenerator.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="This Location SpawnDataGenerator"))
class MASSCOMMUNITYSAMPLE_API UThisLocationSpawnDataGenerator : public UMassEntitySpawnDataGeneratorBase
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
			const int32 EntityCount = int32(Count * EntityType.Proportion / Count);

			// I need an adult... why does (EntityCount > 0 && EntityType.GetEntityConfig() != nullptr) linker error?
			// oh, it's just not exported... 
			if (EntityCount > 0)
			{
				FMassEntitySpawnDataGeneratorResult& Res = Results.AddDefaulted_GetRef();
				Res.NumEntities = EntityCount;
				Res.EntityConfigIndex = i;
				Res.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
				
				Res.SpawnData.InitializeAs<FMassTransformsSpawnData>();
				FMassTransformsSpawnData& Transforms = Res.SpawnData.GetMutable<FMassTransformsSpawnData>();
				Transforms.Transforms.Reserve(Res.NumEntities);
				
				int32 v = 0;
				while (v < EntityCount)
				{
					FTransform& Transform = Transforms.Transforms.AddDefaulted_GetRef();
					Transform = OwnerTransform;
					v++;
				}			
				
			}
		}
		FinishedGeneratingSpawnPointsDelegate.Execute(Results);
	};
};
