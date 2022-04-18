// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/MassZoneGraphPathFollowTask.h"


#include "MassEntityTypes.h"
#include "MassZoneGraphNavigationFragments.h"

#include "MassZoneGraphAnnotationTypes.h"

#include "MassZoneGraphNavigationTypes.h"

#include "MassCommonTypes.h"

#include "MSShowLanePositionTask.generated.h"

// /**
//  * 
//  */
// class MASSSAMPLE_API MSShowLanePositionTask
// {
// public:
// 	MSShowLanePositionTask();
// };


class UMSEntityActorExampleSubsystem;





USTRUCT()
struct MASSSAMPLE_API FShowLanePositionTaskData
{
	GENERATED_BODY()
	

	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassZoneGraphTargetLocation TargetLocation;
};

USTRUCT(meta = (DisplayName = "MS Show Lane Position Task"))
struct MASSSAMPLE_API FShowLanePositionTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FShowLanePositionTaskData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;

	
	TStateTreeExternalDataHandle<UMSEntityActorExampleSubsystem> Path2PathSubsystemHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> MassEntitySubsystemHandle;
	TStateTreeExternalDataHandle<UZoneGraphSubsystem> ZoneGraphSubsystemHandle;

	TStateTreeInstanceDataPropertyHandle<FMassZoneGraphTargetLocation> TargetLocationHandle;

	TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> ZoneGraphLaneLocationFragmentHandle;

	

};