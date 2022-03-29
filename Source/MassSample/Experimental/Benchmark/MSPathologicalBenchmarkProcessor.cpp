// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPathologicalBenchmarkProcessor.h"

//This is just to double check my math works out, we don't need it at all.
 void UMSPathologicalBenchmarkProcessor::CombinationsRecursive(UMassEntitySubsystem* EntitySubsystem, int32 length,int32 offset = 0 )
 {
 	if (length ==0)
 	{
 		FString output;
 		for (const auto ScriptStruct : Combinations)
 		{
 			if(ScriptStruct !=nullptr) output+=ScriptStruct->GetName();
 		}

 		++NumCombinations;
 		
 		//UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor: %s,%i"),*output,Combinations.Num());
 		return;
 	}
 	for(int i = offset; i<=Provinces.Num() - length; ++i)
 	{
 		Combinations.Add(Provinces[i]);
 		CombinationsRecursive(EntitySubsystem,length-1,i+1);
 		Combinations.RemoveAt(Combinations.Num()-1);
			
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
	


	for(int i = 1; i<=10; ++i)
	{
		Combinations.Empty();
		CombinationsRecursive(EntitySubsystem,i);
	}
 	
 	

 	//UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor done! Archetypes possible: %i"),Archetypes.Num());



	//create 1000 entities for now
 	int v = 0;

 	for(int i = 0; i<100; ++i)
 	{
 		for (auto Fragment : Provinces)
 		{
 			//For now, we don't actually add the province fragment to the entity
 			EntitySubsystem->CreateEntity(EntitySubsystem->CreateArchetype({FPathologicFragment::StaticStruct()}));
 			++v;
 		}
 	}
 	
 	UE_LOG( LogTemp, Warning, TEXT("MSPathologicalBenchmarkProcessor done! Entities created: %i"),v);
 	
}



 void UMSPathologicalBenchmarkProcessor::ConfigureQueries()
 {
	//quick and dirty way to disable this for now if you want
 	if constexpr (false)
 	{
 		//ideally this would be a tag but that appears to break the query?
 		PathologicQuery.AddRequirement<FPathologicFragment>(EMassFragmentAccess::ReadWrite);
 		// PathologicQuery.AddRequirement(FAlberta::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FBritishColumbia::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FManitoba::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FNewBrunswick::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FNewfoundlandandLabrador::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FNovaScotia::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FOntario::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FPrinceEdwardIsland::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FQuebec::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 		// PathologicQuery.AddRequirement(FSaskatchewan::StaticStruct(),EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
 	}

 	
	 //FIXME: this triggers validity errors on the query internally?
	 // for (auto Fragment : Provinces)
	 // {
		//  PathologicQuery.AddRequirement(Fragment,EMassFragmentAccess::ReadWrite,EMassFragmentPresence::Optional);
	 // }
 }

 void UMSPathologicalBenchmarkProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
 {

 	QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PathologicalBenchmark);

 	PathologicQuery.ForEachEntityChunk(EntitySubsystem,Context,
		 [&,this](FMassExecutionContext& Context)
		 {
		 	
		 	const int32 QueryLength = Context.GetNumEntities();

		 	for (int32 i = 0; i < QueryLength; ++i)
		 	{

		 		auto entity = Context.GetEntity(i);

		 		//I cannot think of a better way to do this due to them being templates!
		 		//Macro time?
		 		if(FMath::RandBool())
		 		{
		 			Context.Defer().AddFragment<FAlberta>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FBritishColumbia>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FManitoba>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FNewBrunswick>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FNewfoundlandandLabrador>(entity);
		 		}	
		 		if(FMath::RandBool())
		 		{
		 			Context.Defer().AddFragment<FNovaScotia>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FOntario>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FPrinceEdwardIsland>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FQuebec>(entity);
		 		}
		 		if(FMath::RandBool())
		 		{
					 Context.Defer().AddFragment<FSaskatchewan>(entity);
		 		}
		 	}
		 });

 	

 	
 }
