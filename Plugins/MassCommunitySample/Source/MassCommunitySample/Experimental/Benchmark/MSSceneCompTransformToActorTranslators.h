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
	// According to him the first two steps (comp to world and updatebounds) can be threaded but not the render dirty
	FORCEINLINE static void SetWorldTransformFastPath(USceneComponent* InComp, const FTransform& InTransform)
	{
		// directly set transform and update bounds 
		InComp->SetComponentToWorld(InTransform);
		InComp->UpdateBounds();
		
		// Evil temp physics set with static cast
		auto bodyinstance = static_cast<UPrimitiveComponent*>(InComp)->BodyInstance;
		FChaosEngineInterface::SetGlobalPose_AssumesLocked(bodyinstance.ActorHandle, InTransform);

		// dirty the render transform 
		InComp->MarkRenderTransformDirty();
		
		for (auto Component : InComp->GetAttachChildren())
		{
			// This * transforms from local space to world space!
			FTransform CompWorldTransform = Component->GetRelativeTransform() * InTransform;

			//These are to support non-relative transforms (could probably omit as this is rare?)

			if(UNLIKELY(InComp->IsUsingAbsoluteLocation()))
			{
				CompWorldTransform.CopyTranslation(InTransform);
			}
			
			if(UNLIKELY(InComp->IsUsingAbsoluteRotation()))
			{
				CompWorldTransform.CopyRotation(InTransform);
			}
			
			if(UNLIKELY(InComp->IsUsingAbsoluteScale()))
			{
				CompWorldTransform.CopyScale3D(InTransform);
			}

			// Recursive!
			SetWorldTransformFastPath(Component,CompWorldTransform);
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
