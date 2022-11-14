// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTemplateRegistry.h"
#include "MassTranslator.h"
#include "MSSceneCompTransformToActorTranslators.generated.h"


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSMassTransformToSceneCompFastPathTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSTransformToSceneCompTranslatorFastPath : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMSTransformToSceneCompTranslatorFastPath();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;


	// Thanks to vblanco for this fast transform setting trick
	// According to him the first two steps (to world and updatebounds) can be threaded but not the render dirty
	FORCEINLINE static void SetWorldTransformFastPath(USceneComponent* InComp, FTransform InTransform)
	{
        
		InComp->SetComponentToWorld(InTransform);
		InComp->UpdateBounds();
		InComp->MarkRenderTransformDirty();
		for (auto Component : InComp->GetAttachChildren())
		{
			// This * transforms from local space to world space!
			InTransform = Component->GetRelativeTransform() * InTransform;

			//These are to support non-relative transforms (could probably omit as this is rare?)
			if(InComp->IsUsingAbsoluteLocation())
			{
				InTransform.CopyTranslation(InTransform);
			}

			if(InComp->IsUsingAbsoluteRotation())
			{
				InTransform.CopyRotation(InTransform);
			}

			if(InComp->IsUsingAbsoluteScale())
			{
				InTransform.CopyScale3D(InTransform);
			}

			// Recursive!
			SetWorldTransformFastPath(Component,InTransform);
		}
	};

	FMassEntityQuery EntityQuery;
};


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSSceneComponentTransformToMassTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSceneCompTransformToMassTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMSSceneCompTransformToMassTranslator();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
