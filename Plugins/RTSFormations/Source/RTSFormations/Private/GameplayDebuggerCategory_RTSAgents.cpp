// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayDebuggerCategory_RTSAgents.h"

#include "DrawDebugHelpers.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameFramework/PlayerController.h"

FGameplayDebuggerCategory_RTSAgents::FGameplayDebuggerCategory_RTSAgents()
{
	bShowOnlyWithDebugActor = false;
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_RTSAgents::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (OwnerPC)
	{
		//DataPack.ActorName = OwnerPC->GetPawn()->GetName();
		URTSFormationSubsystem* FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(OwnerPC->GetWorld());
		UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(OwnerPC->GetWorld());
		DataPack.NumUnits = FormationSubsystem->Units.Num();

		DataPack.Positions.Empty();
		DataPack.Positions.Reserve(FormationSubsystem->Units[0].Entities.Num());
		
		for(auto& Entity : FormationSubsystem->Units[0].Entities)
		{
			const FVector& Pos = EntitySubsystem->GetFragmentDataChecked<FMassMoveTargetFragment>(Entity).Center;
			DataPack.Positions.Add(Pos);
		}
	}
}

void FGameplayDebuggerCategory_RTSAgents::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("{yellow}Units: {white}%d"), DataPack.NumUnits);
	CanvasContext.Printf(TEXT("{yellow}Entities in Unit 0: {white}%d"), DataPack.Positions.Num());
	for(FVector& Pos : DataPack.Positions)
	{
		DrawDebugCanvasWireSphere(CanvasContext.Canvas.Get(), Pos, FColor::Yellow, 20.f, 5);
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_RTSAgents::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_RTSAgents());
}

void FGameplayDebuggerCategory_RTSAgents::FRepData::Serialize(FArchive& Ar)
{
	Ar << Positions;
	Ar << NumUnits;
}

#endif // WITH_GAMEPLAY_DEBUGGER
