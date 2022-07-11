// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassProcessor.h"
#include "Common/Fragments/MSFragments.h"
#include "MSDebugPrintProcessors.generated.h"
/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSDebugPrintProcessors : public UMassProcessor
{
	GENERATED_BODY()

protected:
	// FIXMEFUNK: Please try to not add much code in headers, let's try to follow a convention.

	virtual void ConfigureQueries() override
	{
		DebugPointDisplay.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
		DebugPointDisplay.AddTagRequirement<FMassSampleDebuggableTag>(EMassFragmentPresence::All);
	}

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override
	{
		DebugPointDisplay.ForEachEntityChunk(EntitySubsystem, Context, [&, this](FMassExecutionContext& Context)
		{
#if WITH_EDITOR
			const auto Transforms = Context.GetFragmentView<FTransformFragment>().GetData();

			auto Entity = Context.GetEntity(0);
			FStringOutputDevice Description;
			EntitySubsystem.DebugGetStringDesc(EntitySubsystem.GetArchetypeForEntity(Entity), Description);

			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				auto Location = Transforms[i].GetTransform().GetTranslation();
				//UE_VLOG_LOCATION(this, LogTemp, Verbose, Transforms[i].Transform.GetTranslation(), 10, FColor::MakeRandomColor(), TEXT("%s"), *description);
				DrawDebugString(GetWorld(), Location, static_cast<FString>(Description), 0, FColor::White, 0, true);
			}
#endif
		});
	}

	FMassEntityQuery DebugPointDisplay;
};
