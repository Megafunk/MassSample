// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMassExecutorExamples.h"
#include "MassSignalSubsystem.h"
#include "Fragments/MSFragments.h"


// New FQueryExecutor template for defining queries and execution functions
// This one is used in a signal processor which requires some cheese in the the ConfigureQueries override (but not in this tempplate) 
// Using them for normal processors will be much less weird
// The important thing is setting UMassProcessor::AutoExecuteQuery and it being picked up in ConfigureQueries
struct FTestQueryExecutor_ForSignal : public UE::Mass::FQueryExecutor
{
	// Read-only access to FTransformFragment
	UE::Mass::FQueryDefinition<
		UE::Mass::FConstFragmentAccess<FTransformFragment>
	> Accessors{*this};

	// Basically just ::Execute but in a function ptr!
	virtual void Execute(FMassExecutionContext& Context)
	{
		// Special ForEachEntityChunk that uses the Accessors above
		ForEachEntityChunk(Context,
		                   Accessors,
		                   [](FMassExecutionContext& Context, auto& Data)
		                   {
			                   for (uint32 EntityIndex : Context.CreateEntityIterator())
			                   {
			                   	// 
				                   const FTransformFragment& TransformFragment = Data.template Get<FTransformFragment>()[EntityIndex];
				                   DrawDebugSphere(Context.GetWorld(),
				                                   TransformFragment.GetTransform().GetLocation(),
				                                   10.0f,
				                                   8,
				                                   FColor::Red,
				                                   false,
				                                   1.0f);
			                   }
		                   });
	}
};


UMSSignalProcessorExecutorExample::UMSSignalProcessorExecutorExample()
{
}

void UMSSignalProcessorExecutorExample::ConfigureQueries(const TSharedRef<FMassEntityManager>& MassEntityManager)
{
	// I am unsure if this needs to be set in here or earlier but this appears to be fine? I think it just needs to exist before Initialize
	AutoExecuteQuery = UE::Mass::FQueryExecutor::CreateQuery<FTestQueryExecutor_ForSignal>(EntityQuery, this);

	// cheesy fix for the engine not calling super for signal processors
	// We call the UMassProcessor version directly because we have to call  AutoExecuteQuery->ConfigureQuery
	UMassProcessor::ConfigureQueries(MassEntityManager);
}

void UMSSignalProcessorExecutorExample::SignalEntities(FMassEntityManager& EntityManager,
                                                       FMassExecutionContext& Context,
                                                       FMassSignalNameLookup& EntitySignals)
{
	AutoExecuteQuery->Execute(Context);
}

void UMSSignalProcessorExecutorExample::OnSignalReceived(FName SignalName, TConstArrayView<FMassEntityHandle> Entities)
{
	Super::OnSignalReceived(SignalName, Entities);
}


void UMSSignalProcessorExecutorExample::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	// Subscribe to OnEntityHitSomething

	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	if (ensureMsgf(SignalSubsystem, TEXT("UMSSignalProcessorExecutorExample Needs the UMassSignalSubsystem available!")))
	{
		SubscribeToSignal(*SignalSubsystem, MassSample::Signals::OnEntityHitSomething);
	}
}
