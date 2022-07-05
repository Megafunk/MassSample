#include "ISMPerInstanceDataTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "Common/Fragments/MSFragments.h"

void UISMPerInstanceDataTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FISMPerInstanceDataFragment>();
}
