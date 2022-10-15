#pragma once

#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MassVisualizationTrait.h"

#include "MSBasicVisualizationTrait.generated.h"


USTRUCT()
struct FBasicVisualizationTag : public FMassTag
{
	GENERATED_BODY()
	
};
 

UCLASS(meta=(DisplayName="Sample Visualization"))
class MASSCOMMUNITYSAMPLE_API UMSBasicVisualizationTrait : public UMassVisualizationTrait
{
	GENERATED_BODY()
public:
	UMSBasicVisualizationTrait();
protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
