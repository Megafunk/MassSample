// Fill out your copyright notice in the Description page of Project Settings.


#include "MSEntityViewBenchMark.h"
#include "MassCommonTypes.h"

#include "MassEntityView.h"



UMSEntityViewBenchMark::UMSEntityViewBenchMark()
{
	int shouldregister = -1;
	if(FParse::Value(FCommandLine::Get(), TEXT("ViewBenchmarkCount="), shouldregister))
	{
		bAutoRegisterWithProcessingPhases = static_cast<bool>(shouldregister);	

	};

	
}

void UMSEntityViewBenchMark::Initialize(UObject& Owner)
{

	FEntityViewBenchmarkFragment Fragment;

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	int EntityCount = 5000;
	FParse::Value(FCommandLine::Get(), TEXT("ViewBenchmarkCount="), EntityCount);


	
	for(int i = 0; i<EntityCount; ++i)
	{
		// Add either tag 1 or tag 2

		const int randomint = FMath::RandRange(1,4);
		const auto entityHandle = EntitySubsystem->CreateEntity({FStructView::Make(Fragment)});

		switch (randomint)
		{
			case 1:
				EntitySubsystem->AddTagToEntity(entityHandle,FEntityViewBenchmarkTag1::StaticStruct());

			break;
			case 2:
				EntitySubsystem->AddTagToEntity(entityHandle,FEntityViewBenchmarkTag2::StaticStruct());

			break;
			case 3:
				EntitySubsystem->AddTagToEntity(entityHandle,FEntityViewBenchmarkTag3::StaticStruct());

			break;
			case 4:
				EntitySubsystem->AddTagToEntity(entityHandle,FEntityViewBenchmarkTag4::StaticStruct());

			break;
		}


		
	}

}

void UMSEntityViewBenchMark::ConfigureQueries()
{
	EntityViewQuery.AddRequirement<FEntityViewBenchmarkFragment>(EMassFragmentAccess::ReadOnly);

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;

}

void UMSEntityViewBenchMark::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{


	{
		FPlatformMisc::MemoryBarrier();

		QUICK_SCOPE_CYCLE_COUNTER(EntityViewBenchMarkEntityView);

		EntityViewQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
		{

			const int32 QueryLength = Context.GetNumEntities();

			auto Fragments = Context.GetFragmentView<FEntityViewBenchmarkFragment>();

			for (int32 i = 0; i < QueryLength; ++i)
			{
				FMassEntityView EntityView = FMassEntityView(EntitySubsystem, Context.GetEntity(i));
			
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
		});
		FPlatformMisc::MemoryBarrier();

	}





	//just to force the compiler to actually do something
	if(Counter >= 10000000)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMSEntityViewBenchMark counter %i!"), Counter);
		Counter = 0;
	}
}



UMSEntityViewWrapperBenchMark::UMSEntityViewWrapperBenchMark()
{

	int shouldregister = -1;
	if(FParse::Value(FCommandLine::Get(), TEXT("ViewBenchmarkCount="), shouldregister))
	{
		bAutoRegisterWithProcessingPhases = static_cast<bool>(shouldregister);	

	};	
}

void UMSEntityViewWrapperBenchMark::ConfigureQueries()
{
	EntityWrapperQuery.AddRequirement<FEntityViewBenchmarkFragment>(EMassFragmentAccess::ReadOnly);

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;

}

void UMSEntityViewWrapperBenchMark::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{


	{
		FPlatformMisc::MemoryBarrier();

		QUICK_SCOPE_CYCLE_COUNTER(EntityViewBenchMarkViewWrapper);

		EntityWrapperQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
			{

				const int32 QueryLength = Context.GetNumEntities();

				auto Fragments = Context.GetFragmentView<FEntityViewBenchmarkFragment>();

				for (int32 i = 0; i < QueryLength; ++i)
				{
					FMassEntityWrapper EntityWrapper = FMassEntityWrapper(&EntitySubsystem, Context.GetEntity(i));
			
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
			});
		FPlatformMisc::MemoryBarrier();

	}





	//just to force the compiler to actually do something
	if(Counter >= 10000000)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMSEntityViewBenchMark counter %i!"), Counter);
		Counter = 0;
	}
}








