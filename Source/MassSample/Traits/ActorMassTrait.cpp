// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorMassTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Fragments/MSFragments.h"

void UActorMassTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	FMassActorFragment& MassActorFragment = BuildContext.AddFragment_GetRef<FMassActorFragment>();
	
	//@TODO: This one is really pushing it! man...
	MassActorFragment.SetNoHandleMapUpdate(FMassEntityHandle(), Actor, bIsOwnedByMass);
	
	
}

UActorMassInitializer::UActorMassInitializer()
{
	ObservedType = FMassActorFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UActorMassInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);

}

void UActorMassInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{

				const TArrayView<FMassActorFragment> ActorsList = Context.GetMutableFragmentView<FMassActorFragment>();

				for (int32 i = 0; i < Context.GetNumEntities(); ++i)
				{
					const FMassEntityHandle Entity = Context.GetEntity(i);

					FMassActorFragment& ActorFragment = ActorsList[i];

					if(IsValid(ActorFragment.GetMutable()))
					{
						ActorFragment.SetAndUpdateHandleMap(Entity,ActorFragment.GetMutable(),ActorFragment.IsOwnedByMass());

					}

					
				}
				
		
	});
	
}
