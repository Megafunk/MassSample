// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommandBuffer.h"
#include "MSDeferredCommands.generated.h"

/**
* Experimental command for building an entity with fragment instances and tag types. Includes optional shared values 
*/
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FBuildEntityFromFragmentInstancesAndTags : public FCommandBufferEntryBase
{
	GENERATED_BODY()
	enum
	{
		Type = ECommandBufferOperationType::Add
	};

	FBuildEntityFromFragmentInstancesAndTags() = default;
	FBuildEntityFromFragmentInstancesAndTags(const FMassEntityHandle Entity, TConstArrayView<FInstancedStruct> InInstances, const TArray<const UScriptStruct*> InTags, FMassArchetypeSharedFragmentValues InSharedFragmentValues = {})
		: FCommandBufferEntryBase(Entity)
		, FragmentInstancesToAdd(InInstances)
		, TagsToAdd(InTags)
		, SharedFragmentValuesToAdd(InSharedFragmentValues)
	{}

	void AppendAffectedEntitiesPerType(FMassCommandsObservedTypes& ObservedTypes);

protected:
	virtual void Execute(UMassEntitySubsystem& EntitySystem) const override;


	TArray<FInstancedStruct> FragmentInstancesToAdd;
	
	TArray<const UScriptStruct*> TagsToAdd;

	FMassArchetypeSharedFragmentValues SharedFragmentValuesToAdd;
};
