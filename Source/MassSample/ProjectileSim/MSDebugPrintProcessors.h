// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSProjectileSubsystem.h"

#include "MassCommonFragments.h"
#include "MassProcessor.h"
#include "Fragments/MSFragments.h"
#include "MassGameplayDebug/Public/MassGameplayDebugTypes.h"
#include "VisualLogger/VisualLogger.h"
#include "MSDebugPrintProcessors.generated.h"
/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSDebugPrintProcessors : public UMassProcessor
{
	GENERATED_BODY()
protected:

	virtual void ConfigureQueries() override
	{
		DebugPointDisplay.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
		DebugPointDisplay.AddTagRequirement<FMassSampleDebuggableTag>(EMassFragmentPresence::All);


	}
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override
	{
		DebugPointDisplay.ForEachEntityChunk(EntitySubsystem,Context,[&,this](FMassExecutionContext& Context)
		{
			const auto Transforms = Context.GetFragmentView<FTransformFragment>().GetData();
			
			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				auto e = Context.GetEntity(i);
				FStringOutputDevice description;
				EntitySubsystem.DebugGetStringDesc(EntitySubsystem.GetArchetypeForEntity(e),description);
				auto location = Transforms[i].GetTransform().GetTranslation();
				//UE_VLOG_LOCATION(this, LogTemp, Verbose, Transforms[i].Transform.GetTranslation(), 10, FColor::MakeRandomColor(), TEXT("%s"), *description);

				DrawDebugString(GetWorld(),location,static_cast<FString>(description),0,FColor::White,0,true);

			}
		});
	}
	
	FMassEntityQuery DebugPointDisplay;

};
