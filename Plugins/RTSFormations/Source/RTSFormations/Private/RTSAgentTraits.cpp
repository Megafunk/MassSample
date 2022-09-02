// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTraits.h"

#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassObserverRegistry.h"
#include "Engine/World.h"

void URTSFormationAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	check(EntitySubsystem);
	
	BuildContext.AddFragment<FRTSFormationAgent>();
	
	uint32 MySharedFragmentHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(FormationSettings));
	FSharedStruct MySharedFragment = EntitySubsystem->GetOrCreateSharedFragment<FRTSFormationSettings>(MySharedFragmentHash, FormationSettings);
	BuildContext.AddSharedFragment(MySharedFragment);
}
