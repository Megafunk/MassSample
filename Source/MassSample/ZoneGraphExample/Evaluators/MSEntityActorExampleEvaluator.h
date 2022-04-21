// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "UObject/NoExportTypes.h"
#include "MSEntityActorExampleEvaluator.generated.h"


class UMassAgentSubsystem;
class UMassEntitySubsystem;
class UMassActorSubsystem;
class UMassSignalSubsystem;
class UCapsuleComponent;
class UMSEntityActorExampleSubsystem;


USTRUCT()
struct MASSSAMPLE_API FMSEntityActorExampleEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Output)
	FString TestString;
};

USTRUCT(meta = (DisplayName = "Entity Actor Example Eval"))
struct MASSSAMPLE_API FMSEntityActorExampleEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FMSEntityActorExampleEvaluatorInstanceData::StaticStruct();
	}

	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<UMSEntityActorExampleSubsystem> MSEntityActorExampleSubsystemHandle;


	TStateTreeInstanceDataPropertyHandle<FString> TestStringHandle;
};