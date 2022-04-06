#pragma once

#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MassVisualizationTrait.h"

#include "MSBasicVisualizationTrait.generated.h"



UCLASS(meta=(DisplayName="Sample Visualization"))
class MASSSAMPLE_API UMSBasicVisualizationTrait : public UMassVisualizationTrait
{
	GENERATED_BODY()
	
protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};
