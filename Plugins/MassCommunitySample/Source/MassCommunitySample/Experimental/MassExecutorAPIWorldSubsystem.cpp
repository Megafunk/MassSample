// Fill out your copyright notice in the Description page of Project Settings.


#include "MassExecutorAPIWorldSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassQueryExecutor.h"
#include "MassSimulationSubsystem.h"

static bool GbMSNewAPIProcessorExampleEnabled = false;
static FAutoConsoleVariableRef CVarMSNewAPIProcessorExample(
	TEXT("masssample.debugoctree"), GbMSNewAPIProcessorExampleEnabled,
	TEXT("see UMassExecutorAPIWorldSubsystem"),
	ECVF_Default);

void UMassExecutorAPIWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
	
	auto EntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	
	// Collection.InitializeDependency<UMassSimulationSubsystem>();

	ĘntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();

}

// FQueryExecutors are a fancy new way to make queries + execution introduced in 5.6
using namespace UE::Mass;
struct FTestQueryExecutor_DrawTransforms : public FQueryExecutor
{
	FQueryDefinition<FConstFragmentAccess<FTransformFragment>> Accessors{*this};
	
	virtual void Execute(FMassExecutionContext& Context)
	{
		ForEachEntityChunk(Context, Accessors, [](FMassExecutionContext& Context, auto& Data)
		{
			for (const int32 EntityIndex : Context.CreateEntityIterator())
			{
				const FTransformFragment& TransformFragment = Data.template Get<FTransformFragment>()[EntityIndex];
				DrawDebugSphere(Context.GetWorld(), TransformFragment.GetTransform().GetLocation(), 10.0f, 8, FColor::Red, false, 1.0f);
			}
		});
	}
};

void UMassExecutorAPIWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	Super::OnWorldBeginPlay(InWorld);
	
	if (!GbMSNewAPIProcessorExampleEnabled) 
	{
		return;
	}
	
	// Currently all processors have an internal AutoExecuteQuery which we expose to set here. It is currently protected though (argh)
	UMassSampleNewAPIProcessor* Processor = NewObject<UMassSampleNewAPIProcessor>(this);
	// This uses the new query class to create and validate the query
	TSharedPtr<FTestQueryExecutor_DrawTransforms> AutoExecuteQuery = 
		UE::Mass::FQueryExecutor::CreateQuery<FTestQueryExecutor_DrawTransforms>(Processor->EntityQuery, Processor);
	Processor->SetAutoExecuteQuery(AutoExecuteQuery);
	Processor->CallInitialize(this, ĘntityManager.ToSharedRef());

	UMassSimulationSubsystem* SimulationSubsystem = InWorld.GetSubsystem<UMassSimulationSubsystem>();

	SimulationSubsystem->RegisterDynamicProcessor(*Processor);
	
	Processors.Add(Processor);
	
}
