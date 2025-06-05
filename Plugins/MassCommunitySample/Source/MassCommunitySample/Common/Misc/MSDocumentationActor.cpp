// Fill out your copyright notice in the Description page of Project Settings.


#include "MSDocumentationActor.h"
#include "Modules/ModuleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSDocumentationActor)

// todo let's make an editor module for this actor because this sucks
#if WITH_EDITOR
#include "SourceCodeNavigation.h"
#endif

// Sets default values
AMSDocumentationActor::AMSDocumentationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMSDocumentationActor::NavigateToFunctionSource(const FString SymbolName, const FString ModuleName)
{
#if WITH_EDITOR

	FString ModuleFileName = FModuleManager::Get().GetModuleFilename(FName(ModuleName));
	
	ModuleFileName = FPaths::GetBaseFilename(ModuleFileName);
	FSourceCodeNavigation::NavigateToFunctionSourceAsync( SymbolName, ModuleFileName, false);
#endif
	
}
// Called when the game starts or when spawned
void AMSDocumentationActor::BeginPlay()
{
	Super::BeginPlay();
	
}

