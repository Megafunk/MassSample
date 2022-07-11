// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassObserverRegistry.h"
#include "MSSparseStorage.generated.h"

USTRUCT()
struct FSparseStorageMap
{
	GENERATED_BODY()

	//The int32 is a FMassEntityHandle's index
	UPROPERTY(meta = (BaseStruct = "FMassSparseFragment"))
	TMap<int32, FInstancedStruct> StorageMap;

	//todo: FScriptArray instead of FInstancedStruct to avoid allocs
};

USTRUCT()
struct FMassSparseFragment
{
	GENERATED_BODY()
};


USTRUCT()
struct FSparseDamageFragment : public FMassSparseFragment
{
	GENERATED_BODY()
};

//Denotes a Mass 
USTRUCT()
struct FMassSparseFragmentOwner : public FMassFragment
{
	GENERATED_BODY()
};

// My very own lazy sparse ECS intended to be used alongside Mass for super-dynamic gameplay code
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSparseStorage : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Collection.InitializeDependency<UMassEntitySubsystem>();

		MassEntitySystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

		for (TObjectIterator<UStruct> StructIt; StructIt; ++StructIt)
		{
			if (StructIt->IsChildOf(FMassSparseFragment::StaticStruct()))
			{
				SparseFragmentMap.Add(FSparseStorageMap());
				SparseFragmentTypesToIDS.Add(Cast<UScriptStruct>(*StructIt), SparseFragmentMap.Num());
			}
		}
	}

	//Also sets it I guess!
	bool AddSparseFragment(FMassEntityHandle Entity, FInstancedStruct SparseFragment)
	{
		if (!SparseFragment.GetScriptStruct()->IsChildOf(FMassSparseFragment::StaticStruct()) || !Entity.IsValid())
			return false;

		if (MassEntitySystem->GetArchetypeComposition(MassEntitySystem->GetArchetypeForEntity(Entity)).Fragments.
		                      Contains(*FMassSparseFragment::StaticStruct()))
		{
			MassEntitySystem->AddFragmentToEntity(Entity, FMassSparseFragment::StaticStruct());
		}

		uint16 SparseFragmentIndex = *SparseFragmentTypesToIDS.Find(SparseFragment.GetScriptStruct());

		SparseFragmentMap[SparseFragmentIndex].StorageMap.Add(Entity.Index, SparseFragment);

		return true;
	}

	//Also sets it I guess!
	bool RemoveSparseFragment(FMassEntityHandle Entity, FInstancedStruct SparseFragment)
	{
		if (!SparseFragment.GetScriptStruct()->IsChildOf(FMassSparseFragment::StaticStruct()) || !Entity.IsValid())
			return false;

		if (MassEntitySystem->GetArchetypeComposition(MassEntitySystem->GetArchetypeForEntity(Entity)).Fragments.
							  Contains(*FMassSparseFragment::StaticStruct()))
		{
			MassEntitySystem->AddFragmentToEntity(Entity, FMassSparseFragment::StaticStruct());
		}

		uint16 SparseFragmentIndex = *SparseFragmentTypesToIDS.Find(SparseFragment.GetScriptStruct());

		SparseFragmentMap[SparseFragmentIndex].StorageMap.Remove(Entity.Index);

		return true;
	}

	//Yeah, I know... It's worth a try to inline it, okay!!!
	FORCEINLINE void* GetSparseFragmentPtr(FMassEntityHandle Entity, UScriptStruct* SparseFragmentType) const
	{
		if (!SparseFragmentType->IsChildOf(FMassSparseFragment::StaticStruct()) || !Entity.IsValid())
			return false;

		if (MassEntitySystem->GetArchetypeComposition(MassEntitySystem->GetArchetypeForEntity(Entity)).Fragments.
		                      Contains(*FMassSparseFragment::StaticStruct()))
		{
			MassEntitySystem->AddFragmentToEntity(Entity, FMassSparseFragment::StaticStruct());
		}

		uint16 EntityIndex = *SparseFragmentTypesToIDS.Find(SparseFragmentType);

		check(SparseFragmentMap[EntityIndex].StorageMap.Contains(Entity.Index))

		return SparseFragmentMap[EntityIndex].StorageMap.Find(Entity.Index)->GetMutableMemory();
	}

	template <typename T>
	T& GetSparseFragmentData(FMassEntityHandle Entity) const
	{
		static_assert(TIsDerivedFrom<T, FMassSparseFragment>::IsDerived,
			"Pass in a struct derived from FMassSparseFragment next time!!!!!");

		return *static_cast<T*>(GetSparseFragmentPtr(Entity, *T::StaticStruct()));
	}

	UPROPERTY()
	UMassEntitySubsystem* MassEntitySystem;

	UPROPERTY()
	TMap<UScriptStruct*, uint16> SparseFragmentTypesToIDS;

	//page it?
	UPROPERTY()
	TArray<FSparseStorageMap> SparseFragmentMap;
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSparseStorageEntityRegistrationProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSSparseStorageEntityRegistrationProcessor()
	{
		bAutoRegisterWithProcessingPhases = false;
	#if WITH_EDITORONLY_DATA
		bCanShowUpInSettings = false;
	#endif
	}

	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
		
		UMassObserverRegistry::GetMutable().RegisterObserver(
			*FMassSparseFragmentOwner::StaticStruct(),
			EMassObservedOperation::Add, GetClass());
	}

	virtual void ConfigureQueries() override
	{
		EntityQuery.AddRequirement<FMassSparseFragmentOwner>(EMassFragmentAccess::ReadWrite);
	};
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override
	{
		EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
		{
			const int32 QueryLength = Context.GetNumEntities();

			for (int32 i = 0; i < QueryLength; ++i)
			{
			}
		});
	}



	FMassEntityQuery EntityQuery;
};
