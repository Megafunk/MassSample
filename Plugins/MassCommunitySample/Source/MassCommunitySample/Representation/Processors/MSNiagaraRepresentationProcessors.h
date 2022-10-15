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


	void Initialize(UObject& Owner)
	{
		Super::Initialize(Owner);
		//get subsystems here etc..
	}

	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context) override;

	
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
	
	virtual void SignalEntities(FMassEntityManager& EntitySubsystem, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
protected:
	virtual void Initialize(UObject& Owner) override;
};