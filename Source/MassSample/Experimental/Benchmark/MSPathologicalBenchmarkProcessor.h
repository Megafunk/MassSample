// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSPathologicalBenchmarkProcessor.generated.h"



// for example you have 10 components, and each component has a 50% chance of being added to the entity


USTRUCT()
struct MASSSAMPLE_API FAlberta : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};
USTRUCT()
struct MASSSAMPLE_API FBritishColumbia : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};
USTRUCT()
struct MASSSAMPLE_API FManitoba : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};
USTRUCT()
struct MASSSAMPLE_API FNewBrunswick : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};
USTRUCT()
struct MASSSAMPLE_API FNewfoundlandandLabrador : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};



USTRUCT()
struct MASSSAMPLE_API FNovaScotia : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct MASSSAMPLE_API FOntario : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};

USTRUCT()
struct MASSSAMPLE_API FPrinceEdwardIsland : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};USTRUCT()
struct MASSSAMPLE_API FQuebec : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};
USTRUCT()
struct MASSSAMPLE_API FSaskatchewan : public FMassFragment
{
	GENERATED_BODY()
	int64 stuff;
};



// UCLASS()
// class MASSSAMPLE_API UMSPathologicalBenchmarkProcessor : public UMassProcessor
// {
// 	GENERATED_BODY()
// public:
// 	virtual void Initialize(UObject& Owner) override;
// 	//static void Combinations(TArray<UScriptStruct*> array, TArray<UScriptStruct*> result,int32 length );
// protected:
// 	virtual void ConfigureQueries() override;
// 	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
//
// };
