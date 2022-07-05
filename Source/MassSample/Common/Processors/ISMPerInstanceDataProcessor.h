#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ISMPerInstanceDataProcessor.generated.h"

/**
 * 
 */
UCLASS()
class UISMPerInstanceDataProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UISMPerInstanceDataProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	void UpdateCustomData(FMassExecutionContext& Context);
private:
	FMassEntityQuery EntityQuery;
};
