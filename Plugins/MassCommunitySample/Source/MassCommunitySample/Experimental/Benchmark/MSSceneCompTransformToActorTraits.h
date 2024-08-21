// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassAgentTraits.h"
#include "MassEntityView.h"
#include "MassGameplayDebugTypes.h"
#include "MSSceneCompTransformToActorTranslators.h"
#include "Translators/MassSceneComponentLocationTranslator.h"
#include "VisualLogger/VisualLogger.h"
#include "MSSceneCompTransformToActorTraits.generated.h"

/**
 * 
 */
// This is overriden so I can yoink in FMassSceneComponentWrapperFragment without having it exported?
// Mass category hidden to make sure nobody touches the sync direction yet?
UCLASS(HideCategories=(Mass))
class MASSCOMMUNITYSAMPLE_API UMSAgentTransformSyncTraitFastPath : public UMassAgentSyncTrait
{
	GENERATED_BODY()
public:
	UMSAgentTransformSyncTraitFastPath()
	{
		SyncDirection = EMassTranslationDirection::MassToActor;
	}
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override
	{
		BuildContext.AddFragment<FMassSceneComponentWrapperFragment>();
		BuildContext.AddFragment<FTransformFragment>();

		BuildContext.GetMutableObjectFragmentInitializers().Add([=](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
			{
				AActor* AsActor = Cast<AActor>(&Owner);
				if (AsActor && AsActor->GetRootComponent())
				{
					USceneComponent* Component = AsActor->GetRootComponent();
					FMassSceneComponentWrapperFragment& ComponentFragment = EntityView.GetFragmentData<FMassSceneComponentWrapperFragment>();
					ComponentFragment.Component = Component;

					FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();

					REDIRECT_OBJECT_TO_VLOG(Component, &Owner);
					UE_VLOG_LOCATION(&Owner, LogMass, Log, Component->GetComponentLocation(), 30, FColor::Yellow, TEXT("Initial component location"));
					UE_VLOG_LOCATION(&Owner, LogMass, Log, TransformFragment.GetTransform().GetLocation(), 30, FColor::Red, TEXT("Initial entity location"));

					// the entity is the authority
					if (CurrentDirection == EMassTranslationDirection::MassToActor)
					{
						// We could use the fast path here but I don't really see the point? I could change this later I suppose?
						Component->SetWorldTransform(TransformFragment.GetTransform(), /*bSweep*/true, nullptr, ETeleportType::TeleportPhysics);
					}
					// actor is the authority
					else
					{
						TransformFragment.GetMutableTransform() = Component->GetComponentTransform();
					}
				}
			});
		
		if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::MassToActor))
		{
			BuildContext.AddTranslator<UMSTransformToSceneCompTranslatorFastPath>();
		}
		
		if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::ActorToMass))
		{
			BuildContext.AddTranslator<UMSSceneCompTransformToMassTranslator>();
		}


	};
	
};

