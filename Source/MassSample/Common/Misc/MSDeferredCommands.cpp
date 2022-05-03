// Fill out your copyright notice in the Description page of Project Settings.


#include "MSDeferredCommands.h"


void FBuildEntityFromFragmentInstancesAndTags::AppendAffectedEntitiesPerType(FMassCommandsObservedTypes& ObservedTypes)
{
	for (const FInstancedStruct& Struct : FragmentInstancesToAdd)
	{
		ObservedTypes.FragmentAdded(Struct.GetScriptStruct(), TargetEntity);
	}
	
	for (const auto Tag : TagsToAdd)
	{
		ObservedTypes.TagAdded(Tag, TargetEntity);
	}
}

void FBuildEntityFromFragmentInstancesAndTags::Execute(UMassEntitySubsystem& EntitySystem) const
{
	const FMassTagBitSet MassTagBitSet(TagsToAdd);
		
	FMassSharedFragmentBitSet MassSharedFragmentBitSet;

	for (auto SharedFragment : SharedFragmentValuesToAdd.GetSharedFragments())
	{
		MassSharedFragmentBitSet.Add(*SharedFragment.GetScriptStruct());
	}
	for (auto ConstSharedFragment : SharedFragmentValuesToAdd.GetConstSharedFragments())
	{
		MassSharedFragmentBitSet.Add(*ConstSharedFragment.GetScriptStruct());
	}
		
	// todo: No chunks yet I'm afraid... FMassChunkFragmentBitSet is rarely used anyways... template later?
	const FMassArchetypeCompositionDescriptor CompositionDescriptor(FragmentInstancesToAdd,MassTagBitSet,FMassChunkFragmentBitSet(),MassSharedFragmentBitSet);

		
	const auto Archetype = EntitySystem.CreateArchetype(CompositionDescriptor,SharedFragmentValuesToAdd);

	// todo-documentation: Create is to reserve a new one, BUILD is to use an existing reserved entity.
	EntitySystem.BuildEntity(TargetEntity,Archetype);
	
	EntitySystem.SetEntityFragmentsValues(TargetEntity,FragmentInstancesToAdd);
}
