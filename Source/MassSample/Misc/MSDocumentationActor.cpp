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
	auto MassSampleModuleFileName = FModuleManager::Get().GetModuleFilename( "MassSample" );
	//L"UnrealEditor-MassSample-Win64-DebugGame"
	//L"../../../../../Unreal Projects/MassSample-main/Binaries/Win64/UnrealEditor-MassSample-Win64-DebugGame.dll"
	
	MassSampleModuleFileName = FPaths::GetBaseFilename(MassSampleModuleFileName);

	FSourceCodeNavigation::NavigateToFunctionSourceAsync( SymbolName, MassSampleModuleFileName, false);
}
// Called when the game starts or when spawned
void AMSDocumentationActor::BeginPlay()
{
	Super::BeginPlay();
	
}

