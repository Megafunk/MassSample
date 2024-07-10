// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTSFormations.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_RTSAgents.h"
#endif // WITH_GAMEPLAY_DEBUGGER

#define LOCTEXT_NAMESPACE "FRTSFormationsModule"

void FRTSFormationsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("RTSAgents", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_RTSAgents::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGameAndSimulate, 8);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FRTSFormationsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_GAMEPLAY_DEBUGGER
	//If the gameplay debugger is available, unregister the category
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("RTSAgents");
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRTSFormationsModule, RTSFormations)