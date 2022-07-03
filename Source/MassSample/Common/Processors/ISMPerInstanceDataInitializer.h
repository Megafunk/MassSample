#pragma once
#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"

#include "ISMPerInstanceDataInitializer.generated.h"

/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UISMPerInstanceDataInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UISMPerInstanceDataInitializer();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
private:
	FMassEntityQuery EntityQuery;
};