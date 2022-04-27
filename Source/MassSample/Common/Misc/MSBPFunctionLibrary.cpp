// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBPFunctionLibrary.h"

#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplateRegistry.h"
#include "MassExecutor.h"
#include "MassMovementFragments.h"
#include "MassProcessingPhase.h"
#include "MSSubsystem.h"
#include "AI/NavigationSystemBase.h"
#include "ProjectileSim/Fragments/MSProjectileFragments.h"

FEntityHandleWrapper UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(AActor* Owner, UMassEntityConfigAsset* MassEntityConfig,
                                                                        const UObject* WorldContextObject)
{

	if (!Owner || !MassEntityConfig) return FEntityHandleWrapper();
	
	//todo: who should actually own an entity template? it's probably designed to have just one spawner own it?
	if(const FMassEntityTemplate* EntityTemplate = MassEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*Owner, *MassEntityConfig))
	{
		UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	
		TArray<FMassEntityHandle> SpawnedEntities;

		//Using batch create to trigger observers 
		EntitySubSystem->BatchCreateEntities(EntityTemplate->GetArchetype(), 1, SpawnedEntities);

		const TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate->GetInitialFragmentValues();
		EntitySubSystem->SetEntityFragmentsValues(SpawnedEntities[0], FragmentInstances);
		
		return  FEntityHandleWrapper{SpawnedEntities[0]};
	}
	
	return FEntityHandleWrapper();
}

void UMSBPFunctionLibrary::SetEntityTransform(const FEntityHandleWrapper EntityHandle,const FTransform Transform,const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	check(EntitySubSystem)

	
	if(const auto TransformFragmentPtr =  EntitySubSystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle.Entity))
	{
		TransformFragmentPtr->SetTransform(Transform);
	}
	
}

void UMSBPFunctionLibrary::SetEntityCollisionQueryIgnoredActors(const FEntityHandleWrapper EntityHandle,const TArray<AActor*> IgnoredActors,const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	check(EntitySubSystem)

	
	if(const auto CollisionQueryFragment =  EntitySubSystem->GetFragmentDataPtr<FLineTraceFragment>(EntityHandle.Entity))
	{
		CollisionQueryFragment->QueryParams.AddIgnoredActors(IgnoredActors);
	}
	
}
FTransform UMSBPFunctionLibrary::GetEntityTransform(const FEntityHandleWrapper EntityHandle,const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	check(EntitySubSystem)

	if(!EntitySubSystem->IsEntityValid(EntityHandle.Entity)) return FTransform::Identity;
	
	if(const auto TransformFragmentPtr =  EntitySubSystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle.Entity))
	{
		return TransformFragmentPtr->GetTransform();
	}
	
	return FTransform();
	
}

void UMSBPFunctionLibrary::SetEntityForce(const FEntityHandleWrapper EntityHandle,const FVector Force,const UObject* WorldContextObject)
{
	const UMassEntitySubsystem* EntitySubSystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	
	check(EntitySubSystem)

	if(const auto MassForceFragmentPtr =  EntitySubSystem->GetFragmentDataPtr<FMassForceFragment>(EntityHandle.Entity))
	{
		MassForceFragmentPtr->Value = Force;
	}
	
}



void UMSBPFunctionLibrary::FindHashGridEntitiesInSphere(const FVector Location,const double Radius, TArray<FEntityHandleWrapper>& Entities ,const UObject* WorldContextObject)
{

	QUICK_SCOPE_CYCLE_COUNTER(FindHashGridEntitiesInSphere);

	if(auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		TArray<FMassEntityHandle> EntitiesFound;
		
		int32 numfound = MassSampleSystem->HashGrid.FindPointsInBall(Location,Radius,
		//todo-performance it feels bad to get random entities to query... 
		[&,Location](const FMassEntityHandle Entity)
		{
			const FVector EntityLocation = EntitySystem->GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
			return UE::Geometry::DistanceSquared(Location,EntityLocation);
		},EntitiesFound);

		Entities.Reserve(numfound);
		for (auto EntityFound : EntitiesFound)
		{
			Entities.Add(FEntityHandleWrapper{EntityFound});
		}
		
	}
}

void UMSBPFunctionLibrary::FindClosestHashGridEntityInSphere(const FVector Location,const double Radius, FEntityHandleWrapper& Entity ,const UObject* WorldContextObject,TEnumAsByte<EReturnSuccess>& ReturnBranch)
{

	QUICK_SCOPE_CYCLE_COUNTER(FindCloestHashGridEntityInSphere);

	if(auto MassSampleSystem = WorldContextObject->GetWorld()->GetSubsystem<UMSSubsystem>())
	{
		auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();

		const auto FoundEntityHashMember = MassSampleSystem->HashGrid.FindNearestInRadius(Location, Radius,
			//todo-performance it feels bad to get random entities to query... 
			[&,Location](const FMassEntityHandle Entity)
			{
				const FVector EntityLocation = EntitySystem->GetFragmentDataPtr<FTransformFragment>(Entity)->
				                                             GetTransform().GetLocation();
				return UE::Geometry::DistanceSquared(Location, EntityLocation);
			});

		if(FoundEntityHashMember.Key.IsValid())
		{
			Entity.Entity = FoundEntityHashMember.Key;
			ReturnBranch = EReturnSuccess::Success;
		}
		else
		{
			ReturnBranch = EReturnSuccess::Failure;
		}
	}
}

void UMSBPFunctionLibrary::AddFragmentToEntity(FFragmentWrapper Fragment, FEntityHandleWrapper Entity,
	const UObject* WorldContextObject)
{

	auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	EntitySystem->AddFragmentToEntity(Entity.Entity, Fragment.Fragment.GetScriptStruct());

}

FString UMSBPFunctionLibrary::GetEntityDebugString(FEntityHandleWrapper Entity, const UObject* WorldContextObject)
{
	auto EntitySystem = WorldContextObject->GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySystem)

	if (!Entity.Entity.IsValid())
		return FString();


	FStringOutputDevice OutPut;
	OutPut.SetAutoEmitLineTerminator(true);
	EntitySystem->DebugPrintEntity(Entity.Entity,OutPut);


	return FString{OutPut};
	
}


