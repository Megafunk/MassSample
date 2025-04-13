#pragma once

#include "CoreMinimal.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "MSNiagaraRepresentationProcessors.generated.h"



UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSNiagaraRepresentationProcessors : public UMassProcessor
{
	GENERATED_BODY()
public:
	
	UMSNiagaraRepresentationProcessors();
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;

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
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager) override;
};