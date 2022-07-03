#include "ISMPerInstanceDataProcessor.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "Common/Fragments/MSFragments.h"

UISMPerInstanceDataProcessor::UISMPerInstanceDataProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Representation);
}
void UISMPerInstanceDataProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FISMPerInstanceDataFragment>(EMassFragmentAccess::ReadWrite);
}
void UISMPerInstanceDataProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FISMPerInstanceDataFragment> RenderDatas = Context.GetMutableFragmentView<FISMPerInstanceDataFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		for (int32 EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FISMPerInstanceDataFragment& RenderData = RenderDatas[EntityIdx];
			//put the data you want to pass to the ISM here:)
			RenderData.data = FMath::RandRange(0.f, 1.f);
		}
	}));
}