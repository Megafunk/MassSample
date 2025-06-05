// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassCommonFragments.h"
#include "MassNavigationTypes.h"
#include "MassStateTreeTypes.h"
#include "MSNavMeshMoveTask.generated.h"


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSMassFindNavMeshPathTargetInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Output)
	FMassTargetLocation MoveTargetLocation;
};


// A simple example of how to execute a navmesh query inside of state tree to produce data for a MassTargetLocation
USTRUCT(meta = (DisplayName = "Find NavMesh Wander Target In Radius"))
struct MASSCOMMUNITYSAMPLE_API FMSMassFindNavMeshPathWanderTargetInRadius : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;;
	virtual const UStruct* GetInstanceDataType() const override { return FMSMassFindNavMeshPathTargetInstanceData::StaticStruct(); };

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;;

	// How far to search for a random point
	UPROPERTY(EditAnywhere)
	float Radius = 5000.0f;
	
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;

};