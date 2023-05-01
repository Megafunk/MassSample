// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraActor.h"
#include "NiagaraComponent.h"




void AMSNiagaraActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	GetNiagaraComponent()->SetTickBehavior(ENiagaraTickBehavior::ForceTickLast);
}

