// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPathologicalBenchmarkProcessor.h"

// void UMSPathologicalBenchmarkProcessor::Combinations(TArray<UScriptStruct*> array, TArray<UScriptStruct*> result,int32 length )
// {
// 	if (length==0)
// 	{
//
// 		FString output;
// 		for (auto ScriptStruct : result)
// 		{
// 			output+=ScriptStruct->GetName();
// 		}
// 		UE_LOG( LogTemp, Error, TEXT("MSPathologicalBenchmarkProcessor: %s"),*output);
// 		return;
// 	}
// 	
// 	for (int i = 0; i <= array.Num();++i)
// 	{
// 		result[result.Num()-length] = array[i];
// 		Combinations(array, array, length-1);
// 	}
// }




void UMSPathologicalBenchmarkProcessor::Initialize(UObject& Owner)
{
	// TArray<UScriptStruct*> Provinces = {
	// 	FAlberta::StaticStruct(),
	// 	FBritishColumbia::StaticStruct(),
	// 	FManitoba::StaticStruct(),
	// 	FNewBrunswick::StaticStruct(),
	// 	FNewfoundlandandLabrador::StaticStruct(),
	// 	FNovaScotia::StaticStruct(),
	// 	FOntario::StaticStruct(),
	// 	FPrinceEdwardIsland::StaticStruct(),
	// 	FQuebec::StaticStruct(),
	// 	FSaskatchewan::StaticStruct()};
	//
	//
	// TArray<FMassArchetypeHandle> Archetypes;
	
	//Combinations(Provinces,TArray<UScriptStruct*>{},10);

}



// void UMSPathologicalBenchmarkProcessor::ConfigureQueries()
// {
//
// }
//
// void UMSPathologicalBenchmarkProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
// {
// }
