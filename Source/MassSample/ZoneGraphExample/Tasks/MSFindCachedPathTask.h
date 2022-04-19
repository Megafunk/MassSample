// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MassStateTreeTypes.h"
#include "ZoneGraphTypes.h"

#include "Tasks/MassZoneGraphPathFollowTask.h"
#include "ZoneGraphExample/Fragments/MSZoneGraphExampleFragments.h"

#include "MSFindCachedPathTask.generated.h"



//
// /**
//  * 
//  */
// UCLASS()
// class MASSSAMPLE_API UMSFindCachedPathTask : public UObject
// {
// 	GENERATED_BODY()
// 	
// };


class UZoneGraphAnnotationSubsystem;
class UMassCrowdSubsystem;
class UZoneGraphSubsystem;


USTRUCT()
struct MASSSAMPLE_API FFindCachedPathTaskData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassZoneGraphTargetLocation FollowPathTargetLocation;
	
};



/**
 * 
 */
USTRUCT(meta = (DisplayName = "MS Find Cached Path"))
struct MASSSAMPLE_API FFindCachedPathTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	FFindCachedPathTask();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FFindCachedPathTaskData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;

	
	//TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> LocationHandle;
	TStateTreeExternalDataHandle<UZoneGraphSubsystem> ZoneGraphSubsystemHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> MassEntitySubsystemHandle;
	TStateTreeExternalDataHandle<UZoneGraphAnnotationSubsystem> ZoneGraphAnnotationSubsystemHandle;
	TStateTreeExternalDataHandle<UMassCrowdSubsystem> MassCrowdSubsystemHandle;


	TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> LocationHandle;

	
	TStateTreeInstanceDataPropertyHandle<FMassZoneGraphTargetLocation> FollowPathTargetLocationHandle;

	TStateTreeExternalDataHandle<FZoneGraphPathTestFromFragment> PathFromFragmentHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FZoneGraphTag DisturbanceAnnotationTag = FZoneGraphTag::None;

	
};
