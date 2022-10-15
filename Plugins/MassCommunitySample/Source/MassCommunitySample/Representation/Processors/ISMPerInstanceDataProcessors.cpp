#include "ISMPerInstanceDataProcessors.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "Representation/Fragments/MSRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"

UismPerInstanceDataUpdater::UismPerInstanceDataUpdater()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UismPerInstanceDataUpdater::ConfigureQueries()
{
	EntityQuery.AddRequirement<FSampleISMPerInstanceSingleFloatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);

}

void UismPerInstanceDataUpdater::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
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
				const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIdx];
				FMassInstancedStaticMeshInfo& ISMInfo = ISMInfos[Representation.StaticMeshDescIndex];
				const FSampleISMPerInstanceSingleFloatFragment& RenderData = RenderDatas[EntityIdx];


				// This can accept any struct that the size of n floats. It seems to be required to be called every frame we want to change it
				ISMInfo.AddBatchedCustomData(RenderData.Data, RepresentationLOD.LODSignificance);
			}
		}
	});
}



UISMPerInstanceDataChangerExampleProcessor::UISMPerInstanceDataChangerExampleProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
}
void UISMPerInstanceDataChangerExampleProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FSampleISMPerInstanceSingleFloatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);

}
void UISMPerInstanceDataChangerExampleProcessor::Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
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