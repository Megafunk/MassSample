#include "ISMPerInstanceDataInitializer.h"

#include "MassCommonFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "Common/Fragments/MSFragments.h"

UISMPerInstanceDataInitializer::UISMPerInstanceDataInitializer()
{
	bAutoRegisterWithProcessingPhases = true;
	ObservedType = FISMPerInstanceDataFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UISMPerInstanceDataInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FISMPerInstanceDataFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
}

void UISMPerInstanceDataInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetMutableSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		const FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		const int32 NumEntities = Context.GetNumEntities();

		const TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FISMPerInstanceDataFragment> RenderDatas = Context.GetMutableFragmentView<FISMPerInstanceDataFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const FMassRepresentationFragment& Representation = RepresentationList[EntityIdx];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIdx];
			FMassInstancedStaticMeshInfo& ISMInfo = ISMInfos[Representation.StaticMeshDescIndex];
			const FISMPerInstanceDataFragment& RenderData = RenderDatas[EntityIdx];
			ISMInfo.AddBatchedCustomData(RenderData.data, RepresentationLOD.LODSignificance);
		}
	}));
}
