#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ISMPerInstanceDataProcessors.generated.h"

/**
 * 
 */


UCLASS()
class MASSCOMMUNITYSAMPLE_API UismPerInstanceDataUpdater : public UMassProcessor
{
	GENERATED_BODY()

public:
	UismPerInstanceDataUpdater();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
private:
	FMassEntityQuery EntityQuery;
};


UCLASS()
class MASSCOMMUNITYSAMPLE_API UISMPerInstanceDataChangerExampleProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UISMPerInstanceDataChangerExampleProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
private:
	FMassEntityQuery EntityQuery;
};
