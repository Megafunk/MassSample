#include "ISMPerInstanceDataProcessors.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "Misc/EngineVersionComparison.h"
#include "Representation/Fragments/MSRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ISMPerInstanceDataProcessors)

UismPerInstanceDataUpdater::UismPerInstanceDataUpdater()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UismPerInstanceDataUpdater::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FSampleISMPerInstanceSingleFloatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);

}

void UismPerInstanceDataUpdater::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetMutableSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		const FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		const int32 NumEntities = Context.GetNumEntities();

		const TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FSampleISMPerInstanceSingleFloatFragment> RenderDatas = Context.GetMutableFragmentView<FSampleISMPerInstanceSingleFloatFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const FMassRepresentationFragment& Representation = RepresentationList[EntityIdx];
			if(Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
#if UE_VERSION_OLDER_THAN(5, 4, 0)
				const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIdx];
				FMassInstancedStaticMeshInfo& ISMInfo = ISMInfos[Representation.StaticMeshDescIndex];
				const FSampleISMPerInstanceSingleFloatFragment& RenderData = RenderDatas[EntityIdx];
#else
				const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIdx];
				FMassInstancedStaticMeshInfo& ISMInfo = ISMInfos[Representation.StaticMeshDescHandle.ToIndex()];
				const FSampleISMPerInstanceSingleFloatFragment& RenderData = RenderDatas[EntityIdx];
#endif

				// This can accept any struct that the size of n floats. It seems to be required to be called every frame we want to change it
				ISMInfo.AddBatchedCustomData(RenderData.Data, RepresentationLOD.LODSignificance);
			}
		}
	});
}



UISMPerInstanceDataChangerExampleProcessor::UISMPerInstanceDataChangerExampleProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
}
void UISMPerInstanceDataChangerExampleProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FSampleISMPerInstanceSingleFloatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);

}
void UISMPerInstanceDataChangerExampleProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const TArrayView<FSampleISMPerInstanceSingleFloatFragment> CustomISMDataFragments = Context.GetMutableFragmentView<FSampleISMPerInstanceSingleFloatFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		for (int32 EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FSampleISMPerInstanceSingleFloatFragment& ISMFragment = CustomISMDataFragments[EntityIdx];
			
			// Put the data you want to pass to the ISM here:)
			// Here we just move the number up for fun
			ISMFragment.Data += (.4f  * Context.GetDeltaTimeSeconds());
			if(ISMFragment.Data > 1.0f)
			{
				ISMFragment.Data -= 1.0f;
			}
		}
	});
}
