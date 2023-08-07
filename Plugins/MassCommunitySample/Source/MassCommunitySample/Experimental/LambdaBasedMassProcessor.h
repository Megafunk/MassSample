// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassObserverRegistry.h"
#include "MassProcessor.h"
#include "MSMassUtils.h"
#include "MassSimulationSubsystem.h"
#include "LambdaBasedMassProcessor.generated.h"


/**
 *  A simple Mass processer template to make stuff faster
 *
 *  Currrenlty handles MSMassUtils::Query<T...>  types
 *
 *
 * Example that creates query for FTransformFragment (read/write)FOtherFragment (readonly)
 * Note the const indicating readonly in the template params
 * MSMassUtils::Processor<FTransformFragment, const FOtherFragment>(EntitySim).ForEachChunk([this](FMassExecutionContext& Context)
 *	{
 *		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
 *      auto OtherFragment = Context.GetFragmentView<FOtherFragment>();
 *		for (int32 i = 0; i < Context.GetNumEntities(); i++)
 *		{
 *			// do stuff as normal 
 *		}
 *	});
 *
 * MSMassUtils::GameThreadProcessor for GT only
 *
 *
 *
 * 
 * Observer listening for FOtherFragment added:
 *
 *  * MSMassUtils::Processor<FTransformFragment, const FOtherFragment>(EntitySim).OnAdded<FOtherFragment>([this](FMassExecutionContext& Context)
 *	{
 *		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
 *      auto OtherFragment = Context.GetFragmentView<FOtherFragment>();
 *		for (int32 i = 0; i < Context.GetNumEntities(); i++)
 *		{
 *			// do stuff as normal 
 *		}
 *	});
 *
 *   
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API ULambdaMassProcessor : public UMassProcessor
{
	GENERATED_BODY()

protected:
	ULambdaMassProcessor()
	{
		bAllowMultipleInstances = true;
		bAutoRegisterWithProcessingPhases = false;
		
	};
	virtual void Initialize(UObject& Owner) override
	{
		Super::Initialize(Owner);
	}

	virtual void ConfigureQueries() override
	{
		Query.RegisterWithProcessor(*this);
	};
	FORCEINLINE_DEBUGGABLE virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

public:
	FMassExecuteFunction ExecuteFunction;

	FMassEntityQuery Query;

	template <typename TFragmentObserved>
	void DynamicToObserver(const FMassExecuteFunction& Function, EMassObservedOperation OperationType)
	{
		// we need to unregister this to make it an observer since we already did this after making me! gross!!!
		GetTypedOuter<UMassSimulationSubsystem>()->UnregisterDynamicProcessor(*this);
		FMassObserverManager& ObserverManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager().GetObserverManager();
		ObserverManager.AddObserverInstance(*TFragmentObserved::StaticStruct(), OperationType, *this);
		ExecuteFunction = Function;
	}

	template <typename TFragmentObserved>
	ULambdaMassProcessor& OnAdded(const FMassExecuteFunction& Function)
	{
		DynamicToObserver<TFragmentObserved>(Function, EMassObservedOperation::Add);
		return *this;
	}

	template <typename TFragmentObserved>
	ULambdaMassProcessor& OnRemoved(const FMassExecuteFunction& Function)
	{
		DynamicToObserver<TFragmentObserved>(Function, EMassObservedOperation::Remove);
		return *this;
	}

	FORCEINLINE_DEBUGGABLE ULambdaMassProcessor& ForEachChunk(const FMassExecuteFunction& Function)
	{
		ExecuteFunction = Function;
		return *this;
	}
	// future evil parallel query engine changes only, this won't do anything until then
	FORCEINLINE_DEBUGGABLE ULambdaMassProcessor& ParallelForEachChunk(const FMassExecuteFunction& Function)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		Query.bAllowParallelExecution = true;
#endif
		ExecuteFunction = Function;
		return *this;
	}

	void SetRequiresgameThread(bool bInRequiresGameThread)
	{
		bRequiresGameThreadExecution = bInRequiresGameThread;
	}

	void SetExecutionFlags(EProcessorExecutionFlags NewExecutionFlags)
	{
		ExecutionFlags = static_cast<int32>(NewExecutionFlags);
	}

	ULambdaMassProcessor& Before(const FName BeforeProcessorName)
	{
		ExecutionOrder.ExecuteBefore.Add(BeforeProcessorName);
		return *this;
	}
	ULambdaMassProcessor& After(const FName AfterProcessorName)
	{
		ExecutionOrder.ExecuteAfter.Add(AfterProcessorName);

		return *this;
	}
	ULambdaMassProcessor& InGroup(const FName GroupName)
	{
		ExecutionOrder.ExecuteInGroup =  GroupName;
		return *this;
	}
	ULambdaMassProcessor& Phase(const EMassProcessingPhase Phase)
	{
		ProcessingPhase = Phase;
		return *this;
	}
	
};


namespace MSMassUtils
{

	// Makes a new dynamic processor registered to the entity sim
	inline ULambdaMassProcessor& Processor(UMassSimulationSubsystem* EntitySim, FMassEntityQuery& Query,  bool bRequiresGameThread = false, const FName Name = NAME_None,
	                                       const EProcessorExecutionFlags ExecutionFlags = EProcessorExecutionFlags::All)
	{
		ULambdaMassProcessor* NewProcessor = NewObject<ULambdaMassProcessor>(EntitySim, Name);
		NewProcessor->SetExecutionFlags(ExecutionFlags);
		NewProcessor->SetRequiresgameThread(bRequiresGameThread);

		NewProcessor->Query = Query;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		NewProcessor->Query.bAllowParallelExecution &= !bRequiresGameThread;
#endif
		NewProcessor->Query.RegisterWithProcessor(*NewProcessor);

		EntitySim->RegisterDynamicProcessor(*NewProcessor);
		return *NewProcessor;
	}
	// Makes a new dynamic processor registered to the entity sim with a templated query! See MSMassUtils::Query<>
	template <typename... TFragments>
	ULambdaMassProcessor& Processor(UMassSimulationSubsystem* EntitySim, const FName Name = NAME_None, bool bRequiresGameThread = false,
									const EProcessorExecutionFlags ExecutionFlags = EProcessorExecutionFlags::All)
	{
		FMassEntityQuery Query = MSMassUtils::Query<TFragments...>();
		return MSMassUtils::Processor(EntitySim,Query, bRequiresGameThread,Name,ExecutionFlags);
	}
}

// 100% evil template shenanigans to template GetMutableFragmentView, thanks for Phy, Vblanco, and Bruxisma for this
// Don't even think about using this lol
// template<typename>
// struct TLambdaMatcher;
// template<typename LambdaStorage, typename... ArgTypes>
// struct TLambdaMatcher<void(LambdaStorage::*)(ArgTypes...) const>
// {
// 	template<typename LambdaType>
// 	static void Call(FMassExecutionContext& Context, LambdaType&& Lambda)
// 	{
// 		CallImpl<LambdaType, typename TRemoveReference<ArgTypes>::Type...>(Context, MoveTemp(Lambda));
// 	}
// 	template<typename LambdaType, typename... NoRefArgTypes>
// 	static void CallImpl(FMassExecutionContext& Context, LambdaType&& Lambda)
// 	{
// 		auto ViewsTuple = MakeTuple(Context.GetMutableFragmentView<NoRefArgTypes>()...);
//
// 		const int32 Num = Context.GetNumEntities();
// 		for (int32 Index = 0; Index < Num; Index++)
// 		{
// 			Lambda(Context, ViewsTuple.template Get<TArrayView<NoRefArgTypes>>()[Index]...);
// 		}
// 	}
// };
//
// template<typename LambdaType>
// static void TemplateBasedLambdaProcessorIteratePhy(FMassExecutionContext& Context, LambdaType&& Lambda)
// {
// 	TLambdaMatcher<decltype(&LambdaType::operator())>::Call(Context, MoveTemp(Lambda));
// }

// example usage:
// MSMassUtils::TemplateBasedLambdaProcessorIteratePhy(Context,[&](FMassVelocityFragment& Velocity ,FTransformFragment& Transform)
// {
// 	FTransform& MutableTransform = Transform.GetMutableTransform();
// 	MutableTransform.SetLocation(Transform.GetTransform().GetLocation() + (Velocity.Value * Context.GetDeltaTimeSeconds()));
// });
