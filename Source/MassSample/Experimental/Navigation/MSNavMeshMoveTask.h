// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassStateTreeTypes.h"
#include "MassMovementTypes.h"
#include "MassNavigationFragments.h"
#include "MSSubsystem.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "MSNavMeshMoveTask.generated.h"

USTRUCT()
struct MASSSAMPLE_API FMassNavMeshPathFollowTaskInstanceData
{ 
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Input)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FMassMovementStyleRef MovementStyle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float SpeedScale = 1.0f;
};
/**
 * 
 */
USTRUCT(meta = (DisplayName = "NavMesh Path Follow"))
struct MASSSAMPLE_API FMassNavMeshPathFollowTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassNavMeshPathFollowTaskInstanceData::StaticStruct(); };
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FAgentRadiusFragment> AgentRadiusHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamsHandle;
	TStateTreeExternalDataHandle<UMSSubsystem> MSSubsystemHandle;

	//todo: find some navigation struct for this
	TStateTreeInstanceDataPropertyHandle<FVector> TargetLocationHandle;
	TStateTreeInstanceDataPropertyHandle<FMassMovementStyleRef> MovementStyleHandle;
	TStateTreeInstanceDataPropertyHandle<float> SpeedScaleHandle;
};


USTRUCT()
struct MASSSAMPLE_API FMassFindNavMeshPathTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	FVector MoveTargetLocation;
};


USTRUCT(meta = (DisplayName = "NavMesh Find Target Tester Thingy"))
struct MASSSAMPLE_API FMassFindNavMeshPathTarget : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override
	{

		Linker.LinkInstanceDataProperty(TargetLocationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassFindNavMeshPathTargetInstanceData, MoveTargetLocation));

		return true;

	};
	virtual const UStruct* GetInstanceDataType() const override { return FMassFindNavMeshPathTargetInstanceData::StaticStruct(); };
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override
	{
		Context.GetInstanceData(TargetLocationHandle) = TargetLocation;

		return EStateTreeRunStatus::Running;
	};
	
	TStateTreeInstanceDataPropertyHandle<FVector> TargetLocationHandle;
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	FVector TargetLocation;
	
};