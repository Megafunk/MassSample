// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSPathologicalBenchmarkProcessor.generated.h"

/*
 *
 *This is an ECS benchmark called Pathological invented by vblanco
 *It creates entities with a random sets of components as a stress test.
 *We start with 10 different components and create combinations of each from length 1 to 10.
 *Then we creates a massive list of 1023 different archetypes
 *This one of the worst performance cases for archetype style ECS implementations
 */
USTRUCT()
struct FAlberta : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FBritishColumbia : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FManitoba : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FNewBrunswick : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FNewfoundlandandLabrador : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FNovaScotia : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FOntario : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FPrinceEdwardIsland : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FQuebec : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FSaskatchewan : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct FPathologicFragment : public FMassFragment
{
	GENERATED_BODY()
};

UCLASS()
class MASSSAMPLE_API UMSPathologicalBenchmarkProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	void CombinationsRecursive(UMassEntitySubsystem* EntitySubsystem, int32 length, int32 offset);
	virtual void Initialize(UObject& Owner) override;
	//static void Combinations(TArray<UScriptStruct*> array, TArray<UScriptStruct*> result,int32 length );
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	TArray<UScriptStruct*> Combinations;
	TArray<UScriptStruct*> Provinces;

	int NumCombinations;

	FMassEntityQuery PathologicQuery;
	
};
