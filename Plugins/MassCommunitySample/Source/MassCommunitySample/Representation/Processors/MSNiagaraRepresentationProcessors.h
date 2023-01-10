#pragma once

#include "CoreMinimal.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "UObject/Object.h"
#include "MSNiagaraRepresentationProcessors.generated.h"



UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNiagaraRepresentationProcessors : public UMassProcessor
{
	GENERATED_BODY()
public:
	
	UMSNiagaraRepresentationProcessors();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	
	FMassEntityQuery PositionToNiagaraFragmentQuery;
	FMassEntityQuery PushArraysToNiagaraSystems;
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNiagaraRepresentationSpawnProcs : public UMassSignalProcessorBase
{
	GENERATED_BODY()
public:
	
	UMSNiagaraRepresentationSpawnProcs();
	
	virtual void ConfigureQueries() override;
	
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
protected:
	virtual void Initialize(UObject& Owner) override;
};