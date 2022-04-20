// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraActor.h"

#include "MassEntitySubsystem.h"
#include "MassProcessingPhase.h"
#include "NiagaraComponent.h"


// Sets default values
AMSNiagaraActor::AMSNiagaraActor(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMSNiagaraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMSNiagaraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMSNiagaraActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	GetNiagaraComponent()->SetTickGroup(ETickingGroup::TG_PostPhysics);
}

