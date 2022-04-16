// Fill out your copyright notice in the Description page of Project Settings.


#include "MSDocumentationActor.h"

#include "SourceCodeNavigation.h"
#include "Misc/PathViews.h"

// Sets default values
AMSDocumentationActor::AMSDocumentationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMSDocumentationActor::NavigateToFunctionSource(const FString SymbolName, const FString ModuleName)
{
	FString ModuleFileName = FModuleManager::Get().GetModuleFilename(FName(ModuleName));
	
	ModuleFileName = FPaths::GetBaseFilename(ModuleFileName);
#if WITH_EDITOR
	FSourceCodeNavigation::NavigateToFunctionSourceAsync( SymbolName, ModuleFileName, false);
#endif
	
}
// Called when the game starts or when spawned
void AMSDocumentationActor::BeginPlay()
{
	Super::BeginPlay();
	
}

