// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MSDocumentationActor.generated.h"

UCLASS()
class MASSCOMMUNITYSAMPLE_API AMSDocumentationActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMSDocumentationActor();
	
	UFUNCTION(BlueprintCallable)
	static void NavigateToFunctionSource(const FString SymbolName, const FString ModuleName = "MassSample");

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
