// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPathologicalBenchmarkProcessor.h"


constexpr bool benchmark = false;

UMSPathologicalBenchmarkProcessor::UMSPathologicalBenchmarkProcessor()
{
	if constexpr (!benchmark)
	{

		bAutoRegisterWithProcessingPhases = false;
		
	}
}

//This is just to double check my math works out, we don't need it at all.
 void UMSPathologicalBenchmarkProcessor::CombinationsRecursive(UMassEntitySubsystem* EntitySubsystem, int32 length,int32 offset = 0 )
 {
 	if (length ==0)
 	{
 		FString output;
 		for (const auto ScriptStruct : Combination)
 		{
 			if(ScriptStruct !=nullptr) output+=ScriptStruct->GetName();
 		}

 		++NumCombinations;
 		UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor: %s,%i"),*output,Combination.Num());

 		TArray<UScriptStruct*> CombinationWithPatholigicFragment = Combination;
 		CombinationWithPatholigicFragment.Add(FPathologicFragment::StaticStruct());
 		Archetypes.Add(EntitySubsystem->CreateArchetype({CombinationWithPatholigicFragment}));
 		
 		return;
 	}
 	for(int i = offset; i<=Provinces.Num() - length; ++i)
 	{
 		Combination.Add(Provinces[i]);
 		CombinationsRecursive(EntitySubsystem,length-1,i+1);
 		Combination.Pop();
 		
			
 	}
 }




void UMSPathologicalBenchmarkProcessor::Initialize(UObject& Owner)
 {
	 UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

 	//10 Different fragments!
 	//TODO: let us turn this amount up and down?
 	Provinces = {
 		FAlberta::StaticStruct(),
		 FBritishColumbia::StaticStruct(),
		 FManitoba::StaticStruct(),
		 FNewBrunswick::StaticStruct(),
		 FNewfoundlandandLabrador::StaticStruct(),
		 FNovaScotia::StaticStruct(),
		 FOntario::StaticStruct(),
		 FPrinceEdwardIsland::StaticStruct(),
		 FQuebec::StaticStruct(),
		 FSaskatchewan::StaticStruct()};
	


 	for(int i = 1; i<=Provinces.Num(); ++i)
 	{
 		Combination.Empty();
 		CombinationsRecursive(EntitySubsystem,i);
 	}
 	
 	//the final 1024th archetype I guess?
 	Archetypes.Add(EntitySubsystem->CreateArchetype({FPathologicFragment::StaticStruct()}));


 	UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor done! Archetypes made: %i"),Archetypes.Num());



	//create 100000 entities for now
 	int v = 0;

 	for(int i = 0; i<100000; ++i)
 	{
 		const int32 Index = FMath::RandRange(0, Archetypes.Num() - 1);

	    	EntitySubsystem->CreateEntity(Archetypes[Index]);
 		
 		++v;
 	}
 	
 	UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor done! Entities created: %i"),v);
 	
}



 void UMSPathologicalBenchmarkProcessor::ConfigureQueries()
 {
	//quick and dirty way to disable this for now if you want
 	if constexpr (benchmark)
 	{
 		PathologicQuery9.AddRequirement<FPathologicFragment>(EMassFragmentAccess::ReadWrite);
 		
 		PathologicQuery9.AddRequirement(FAlberta::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FBritishColumbia::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FManitoba::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FNewBrunswick::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FNewfoundlandandLabrador::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FNovaScotia::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FOntario::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FPrinceEdwardIsland::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery9.AddRequirement(FQuebec::StaticStruct(),EMassFragmentAccess::ReadWrite);

 		PathologicQuery3.AddRequirement<FPathologicFragment>(EMassFragmentAccess::ReadWrite);
 		
 		PathologicQuery3.AddRequirement(FAlberta::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery3.AddRequirement(FBritishColumbia::StaticStruct(),EMassFragmentAccess::ReadWrite);
 		PathologicQuery3.AddRequirement(FManitoba::StaticStruct(),EMassFragmentAccess::ReadWrite);


 	}

 	

 }

 void UMSPathologicalBenchmarkProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{

	UE_LOG( LogTemp, Warning, TEXT("PathologicalBenchmark9: %i entities found"),PathologicQuery9.GetNumMatchingEntities(EntitySubsystem));
	UE_LOG( LogTemp, Warning, TEXT("PathologicalBenchmark3: %i entities found"),PathologicQuery3.GetNumMatchingEntities(EntitySubsystem));

	{


		PathologicQuery9.ParallelForEachEntityChunk(EntitySubsystem,Context,
		[&,this](FMassExecutionContext& Context)
			{
			QUICK_SCOPE_CYCLE_COUNTER(PathologicalBenchmark9Loop);

				const int32 QueryLength = Context.GetNumEntities();

				auto Albertas = Context.GetMutableFragmentView<FAlberta>();
				auto BritishColumbias = Context.GetMutableFragmentView<FBritishColumbia>();
				auto Manitobas = Context.GetMutableFragmentView<FManitoba>();
				auto NewBrunswicks = Context.GetMutableFragmentView<FNewBrunswick>();
				auto NewfoundlandandLabradors = Context.GetMutableFragmentView<FNewfoundlandandLabrador>();
				auto NovaScotias = Context.GetMutableFragmentView<FNovaScotia>();
				auto Ontarios = Context.GetMutableFragmentView<FOntario>();
				auto PrinceEdwardIslands = Context.GetMutableFragmentView<FPrinceEdwardIsland>();
				auto Quebecs = Context.GetMutableFragmentView<FQuebec>();

				for (int32 i = 0; i < QueryLength; ++i)
				{

	
					Quebecs[i].stuff += 
					Albertas[i].stuff +
				   BritishColumbias[i].stuff +
				   Manitobas[i].stuff +
				   NewBrunswicks[i].stuff +
				   NewfoundlandandLabradors[i].stuff +
				   NovaScotias[i].stuff +
				   Ontarios [i].stuff +
				   PrinceEdwardIslands[i].stuff +
				   Quebecs[i].stuff;
					
				}
			});

	}
	{

		PathologicQuery3.ParallelForEachEntityChunk(EntitySubsystem,Context,
	   [&,this](FMassExecutionContext& Context)
	   {
	   	QUICK_SCOPE_CYCLE_COUNTER(PathologicalBenchmark3Loop);

		   const int32 QueryLength = Context.GetNumEntities();

		   auto Albertas = Context.GetMutableFragmentView<FAlberta>();
		   auto BritishColumbias = Context.GetMutableFragmentView<FBritishColumbia>();
		   auto Manitobas = Context.GetMutableFragmentView<FManitoba>();


		   for (int32 i = 0; i < QueryLength; ++i)
		   {

			   Albertas[i].stuff +=
			  BritishColumbias[i].stuff +
			  Manitobas[i].stuff;


		   }
	   });
	}
 }
