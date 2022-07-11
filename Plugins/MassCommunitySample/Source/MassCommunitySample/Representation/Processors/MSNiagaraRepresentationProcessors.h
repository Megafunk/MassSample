#pragma once

#include "CoreMinimal.h"
#include "Representation/MSNiagaraSubsystem.h"
#include "MassProcessor.h"
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

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	
	FMassEntityQuery PositionToNiagaraFragmentQuery;
	FMassEntityQuery PushArraysToNiagaraSystems;
};

