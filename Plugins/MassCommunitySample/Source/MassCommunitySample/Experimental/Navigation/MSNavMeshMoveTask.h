// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassStateTreeTypes.h"
#include "MassMovementTypes.h"
#include "MassNavigationFragments.h"
#include "MSNavMeshFragments.h"
#include "MSSubsystem.h"
#include "StateTreeLinker.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "MSNavMeshMoveTask.generated.h"

USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMassNavMeshPathFollowTaskInstanceData
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
struct MASSCOMMUNITYSAMPLE_API FMassNavMeshPathFollowTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassNavMeshPathFollowTaskInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FAgentRadiusFragment> AgentRadiusHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamsHandle;
	TStateTreeExternalDataHandle<FNavMeshAIFragment> NavMeshAIFragmentHandle;

	TStateTreeExternalDataHandle<UMSSubsystem> MSSubsystemHandle;

};


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMassFindNavMeshPathTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	FVector MoveTargetLocation = FVector::ZeroVector;
};


// Super quick task to get a random nav point for testing
USTRUCT(meta = (DisplayName = "Find NavMesh Wander Target In Radius"))
struct MASSCOMMUNITYSAMPLE_API FMassFindNavMeshPathWanderTargetInRadius : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override
	{
		
		Linker.LinkExternalData(TransformHandle);
		return true;
	};
	virtual const UStruct* GetInstanceDataType() const override { return FMassFindNavMeshPathTargetInstanceData::StaticStruct(); };

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;;



	UPROPERTY(EditAnywhere)
	float Radius = 5000.0f;
	
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;

};