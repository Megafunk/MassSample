// Fill out your copyright notice in the Description page of Project Settings.


#include "MSEntityViewBenchMark.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"

#include "MassEntityView.h"


UMSEntityViewBenchMark::UMSEntityViewBenchMark()
{
	bAutoRegisterWithProcessingPhases = false;
	int shouldregister = -1;
	if(FParse::Value(FCommandLine::Get(), TEXT("ViewBenchmarkCount="), shouldregister))
	{
		bAutoRegisterWithProcessingPhases = static_cast<bool>(shouldregister);
	};
}

void UMSEntityViewBenchMark::Initialize(UObject& Owner)
{
	FEntityViewBenchmarkFragment Fragment;

	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	int EntityCount = 5000;
	FParse::Value(FCommandLine::Get(), TEXT("ViewBenchmarkCount="), EntityCount);
	
	for(int i = 0; i<EntityCount; ++i)
	{
		// Add either tag 1 or tag 2

		const int randomint = FMath::RandRange(1,4);
		const auto entityHandle = EntityManager.CreateEntity({FInstancedStruct::Make(Fragment)});

		switch (randomint)
		{
			case 1:
				EntityManager.AddTagToEntity(entityHandle,FEntityViewBenchmarkTag1::StaticStruct());

			break;
			case 2:
				EntityManager.AddTagToEntity(entityHandle,FEntityViewBenchmarkTag2::StaticStruct());

			break;
			case 3:
				EntityManager.AddTagToEntity(entityHandle,FEntityViewBenchmarkTag3::StaticStruct());

			break;
			case 4:
				EntityManager.AddTagToEntity(entityHandle,FEntityViewBenchmarkTag4::StaticStruct());

			break;
		}
	}
}

void UMSEntityViewBenchMark::ConfigureQueries()
{
	EntityViewQuery.AddRequirement<FEntityViewBenchmarkFragment>(EMassFragmentAccess::ReadOnly);
	EntityViewQuery.RegisterWithProcessor(*this);

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
}

void UMSEntityViewBenchMark::BenchA(FMassEntityHandle Entity)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("BenchA"), STAT_EntityView, STATGROUP_MASSSAMPLEVIEW);
	
	FPlatformMisc::MemoryBarrier();

	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	FMassEntityWrapper EntityWrapper = FMassEntityWrapper(EntityManager, Entity);
	
	if(EntityWrapper.HasTag<FEntityViewBenchmarkTag1>())
	{
		++Counter;
	}
	if(EntityWrapper.HasTag<FEntityViewBenchmarkTag2>())
	{
		++Counter;
	}
	if(EntityWrapper.HasTag<FEntityViewBenchmarkTag3>())
	{
		++Counter;
	}
	if(EntityWrapper.HasTag<FEntityViewBenchmarkTag4>())
	{
		++Counter;
	}
}

void UMSEntityViewBenchMark::BenchB(FMassEntityHandle Entity)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("BenchB"), STAT_WrappedEntityView, STATGROUP_MASSSAMPLEVIEWWRAPPED);

	FPlatformMisc::MemoryBarrier();

	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();;

	FMassEntityView EntityView = FMassEntityView(EntityManager, Entity);
			
	if(EntityView.HasTag<FEntityViewBenchmarkTag1>())
	{
		++Counter;
	}
	if(EntityView.HasTag<FEntityViewBenchmarkTag2>())
	{
		++Counter;
	}
	if(EntityView.HasTag<FEntityViewBenchmarkTag3>())
	{
		++Counter;
	}
	if(EntityView.HasTag<FEntityViewBenchmarkTag4>())
	{
		++Counter;
	}
}

void UMSEntityViewBenchMark::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> AllEntitiesList;
	
	EntityViewQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
	{
		AllEntitiesList.Append(TArray<FMassEntityHandle>(Context.GetEntities()));
	});
	
	//randomize the list
	if (AllEntitiesList.Num()>0)
	{
		int32 LastIndex = AllEntitiesList.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 index = FMath::RandRange(i, LastIndex);
			if (i != index)
				AllEntitiesList.SwapMemory(i, index);
		}
	}
	
	for (auto Entity : AllEntitiesList)
	{
		BenchA(Entity);
		BenchB(Entity);
	}
	
	//just to force the compiler to actually do something
	if(Counter >= 10000000)
	{
		UE_LOG(LogTemp, Display, TEXT("UMSEntityViewBenchMark counter %i!"), Counter);
		Counter = 0;
	}
}