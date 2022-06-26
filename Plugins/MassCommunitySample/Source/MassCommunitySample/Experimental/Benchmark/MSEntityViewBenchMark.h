// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UObject/Object.h"
#include "MSEntityViewBenchMark.generated.h"

DECLARE_STATS_GROUP(TEXT("MassSampleView"), STATGROUP_MASSSAMPLEVIEW, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("MassSampleViewWrapped"), STATGROUP_MASSSAMPLEVIEWWRAPPED, STATCAT_Advanced);

USTRUCT()
struct FMassEntityWrapper
{
	// Is this fair to compare with editor binary code?

	GENERATED_BODY()
	

	FMassEntityWrapper() = default;
	FMassEntityWrapper(const UMassEntitySubsystem* InEntitySubsystem, const FMassEntityHandle& Entity):
	EntitySystem(InEntitySubsystem),
	Handle(Entity){};

	template<typename T>
	FORCEINLINE bool HasTag() const
	{
		checkSlow(EntitySystem && Handle.IsValid());
		return EntitySystem->GetArchetypeComposition(EntitySystem->GetArchetypeForEntity(Handle)).Tags.Contains<T>();
	}
	
	FORCEINLINE bool HasTag(const UScriptStruct& TagType) const
	{
		checkSlow(EntitySystem && Handle.IsValid());
		return EntitySystem->GetArchetypeComposition(EntitySystem->GetArchetypeForEntity(Handle)).Tags.Contains(TagType);
	}

	const UMassEntitySubsystem* EntitySystem;
	FMassEntityHandle Handle;

};




USTRUCT()
struct FEntityViewBenchmarkFragment : public FMassFragment
{
	GENERATED_BODY()

	int64 Thing1;
	int64 Thing2;
	int64 Thing3;

};

USTRUCT()
struct FEntityViewBenchmarkTag1 : public FMassTag {GENERATED_BODY()};

USTRUCT()
struct FEntityViewBenchmarkTag2 : public FMassTag {GENERATED_BODY()};

USTRUCT()
struct FEntityViewBenchmarkTag3 : public FMassTag {GENERATED_BODY()};

USTRUCT()
struct FEntityViewBenchmarkTag4 : public FMassTag {GENERATED_BODY()};

/** This benchmark is designed to test the overhead of checking archetype composition data from different codepaths
 *  The goal is to make a new 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSEntityViewBenchMark : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMSEntityViewBenchMark();
	virtual void Initialize(UObject& Owner) override;
protected:
	virtual void ConfigureQueries() override;
	void BenchA(FMassEntityHandle Entity);
	void BenchB(FMassEntityHandle Entity);

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	uint32 Counter;

	FMassEntityQuery EntityViewQuery;
	






};

