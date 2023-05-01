// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraActor.h"
#include "GameFramework/Actor.h"
#include "MSNiagaraActor.generated.h"

UCLASS()
class MASSCOMMUNITYSAMPLE_API AMSNiagaraActor : public ANiagaraActor
{
	GENERATED_BODY()
	virtual void PostRegisterAllComponents() override;
};
