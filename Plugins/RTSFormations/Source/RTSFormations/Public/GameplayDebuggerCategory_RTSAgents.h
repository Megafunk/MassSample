// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "RTSFormationSubsystem.h"

class APlayerController;
class AActor;

class FGameplayDebuggerCategory_RTSAgents : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_RTSAgents();
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
    
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();
    
protected:
	struct FRepData
	{
		// Put all data you want to display here
		TArray<FVector> Positions;
		
		int NumUnits = 0;
        
		void Serialize(FArchive& Ar);
	};
    
	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER
