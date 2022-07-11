#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ISMPerInstanceDataProcessors.generated.h"

/**
 * 
 */



UCLASS()
class MASSSAMPLE_API UismPerInstanceDataChanger : public UMassProcessor
{
	GENERATED_BODY()

public:
	UismPerInstanceDataChanger();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
private:
	FMassEntityQuery EntityQuery;
};


UCLASS()
class UISMPerInstanceDataChangerExampleProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UISMPerInstanceDataChangerExampleProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
private:
	FMassEntityQuery EntityQuery;
};
