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
	
	//virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FAgentRadiusFragment> AgentRadiusHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamsHandle;
	TStateTreeExternalDataHandle<FNavMeshAIFragment> NavMeshAIFragmentHandle;

	TStateTreeExternalDataHandle<UMSSubsystem> MSSubsystemHandle;

	

	//todo: find some navigation struct for this
	TStateTreeInstanceDataPropertyHandle<FVector> TargetLocationHandle;
	TStateTreeInstanceDataPropertyHandle<FMassMovementStyleRef> MovementStyleHandle;
	TStateTreeInstanceDataPropertyHandle<float> SpeedScaleHandle;
};


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMassFindNavMeshPathTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	FVector MoveTargetLocation;
};


// Super quick task to get a random nav point for testing
USTRUCT(meta = (DisplayName = "Find NavMesh Wander Target In Radius"))
struct MASSCOMMUNITYSAMPLE_API FMassFindNavMeshPathWanderTargetInRadius : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override
	{

		Linker.LinkInstanceDataProperty(TargetLocationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassFindNavMeshPathTargetInstanceData, MoveTargetLocation));
		
		Linker.LinkExternalData(TransformHandle);

		return true;

	};
	virtual const UStruct* GetInstanceDataType() const override { return FMassFindNavMeshPathTargetInstanceData::StaticStruct(); };

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override
	{

		auto NavSystem = Cast<UNavigationSystemV1>(Context.GetWorld()->GetNavigationSystem());
		FNavLocation NavLocation;
		const FVector Origin = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();


		// todo-navigation pass in nav property stuff
		NavSystem->GetRandomReachablePointInRadius(Origin,Radius,NavLocation);

		Context.GetInstanceData(TargetLocationHandle) = NavLocation.Location;

		return EStateTreeRunStatus::Running;
	};



	UPROPERTY(EditAnywhere)
	float Radius = 5000.0f;
	
	TStateTreeInstanceDataPropertyHandle<FVector> TargetLocationHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;

};