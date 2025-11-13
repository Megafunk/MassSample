// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTemplateRegistry.h"
#include "MassTranslator.h"
#include "PBDRigidsSolver.h"
#include "Chaos/ChaosScene.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "MSSceneCompTransformToActorTranslators.generated.h"


namespace  MassSample::Actors
{

	// I generally dislike hiding things behind templates but this one is way too verbose otherwise
	using FMassSampleAABBQueue = TQueue<TPair<FPhysicsActorHandle,Chaos::FAABB3>, EQueueMode::Mpsc>;
	
	static void UpdateActorInAccelerationStructureThreaded(const FPhysicsActorHandle& Actor,
		Chaos::ISpatialAcceleration<Chaos::FAccelerationStructureHandle, Chaos::FReal, 3>& SpacialAccel,
		Chaos::FPBDRigidsSolver& Solver, FMassSampleAABBQueue& AccelerationQueue)
	{
		using namespace Chaos;
		// FPhysicsSceneGuardScopedWrite ScopedWrite(SceneSolver->GetExternalDataLock_External());
		if(Actor)
		{
			const FRigidBodyHandle_External& Body_External = Actor->GetGameThreadAPI();
			
			FAABB3 WorldBounds;
			const bool bHasBounds = Body_External.GetGeometry()->HasBoundingBox();
			if(bHasBounds)
			{
				WorldBounds = Body_External.GetGeometry()->BoundingBox().TransformedAABB(FRigidTransform3(Body_External.X(), Body_External.R()));

				// Note that this requires p.Chaos.EnableAsyncInitBody 1!
				FAccelerationStructureHandle AccelerationHandle(Actor->GetParticle_LowLevel());

				// WorldSpaceInflatedBounds or something else? UpdateParticle->WorldSpaceInflatedBounds()
				const bool bNeedsUpdate = bHasBounds && SpacialAccel.NeedUpdateElementIn(AccelerationHandle, WorldBounds, Body_External.SpatialIdx());

				if (bNeedsUpdate)
				{
					AccelerationQueue.Enqueue({Actor, WorldBounds});
				}
			}
		}
	}

		static FChaosScene* GetChaosSceneFromActorHandle(const FPhysicsActorHandle& InHandle)
		{
			if(!InHandle)
			{
				return nullptr;
			}

			Chaos::FPBDRigidsSolver* Solver = InHandle->GetSolver<Chaos::FPBDRigidsSolver>();
			return static_cast<FChaosScene*>(Solver ? Solver->PhysSceneHack : nullptr);
		}

		static bool IsKinematic(Chaos::FRigidBodyHandle_External& BodyHandle)
		{
			return BodyHandle.ObjectState() == Chaos::EObjectStateType::Kinematic;
		}

		static void SetGlobalPoseWhileLockedWithoutAccelerationUpdate(const FPhysicsActorHandle& InActorReference,const FTransform& InNewPose, FMassSampleAABBQueue& AccelerationQueue, bool bAutoWake = true)
		{
			Chaos::FRigidBodyHandle_External& Body_External = InActorReference->GetGameThreadAPI();
			// if (!IsKinematic(InActorReference) && !IsSleeping(InActorReference) && Chaos::FVec3::IsNearlyEqual(InNewPose.GetLocation(), Body_External.X(), SMALL_NUMBER) && Chaos::FRotation3::IsNearlyEqual(InNewPose.GetRotation(), Body_External.R(), SMALL_NUMBER))
			// {
			// 	// if simulating, don't update X/R if they haven't changed. this allows scale to be set on simulating body without overriding async position/rotation.
			// 	return;
			// }

			if (IsKinematic(Body_External))
			{
				// NOTE: SetGlobalPose is a teleport for kinematics. Use SetKinematicTarget_AssumesLocked
				// if the kinematic should calculate its velocity from the transform delta.
				Body_External.SetKinematicTarget(InNewPose);
				Body_External.SetV(FVector::Zero());
				Body_External.SetW(FVector::Zero());
			}

			Body_External.SetX(InNewPose.GetLocation());
			Body_External.SetR(InNewPose.GetRotation());

			Body_External.UpdateShapeBounds();
		
			// Acceleration structure work must be queued
			if (FChaosScene* Scene = GetChaosSceneFromActorHandle(InActorReference))
			{
				UpdateActorInAccelerationStructureThreaded(InActorReference, *Scene->GetSpacialAcceleration(), *Scene->GetSolver(), AccelerationQueue);
			}
		}


	

	// This is not the fastest way to go because it forces repeated changes to the acceleration structure that should be batched
	// The manual way it much more complicated though and I want to keep this from being intimidating to look at
	static void SetPhysicsBodyTransformBatchedWhilePhysicsLocked(USceneComponent* InComp, const FTransform& InTransform, FMassSampleAABBQueue& AccelerationQueue)
	{
		
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(InComp))
		{
			FPhysicsActorHandle ChaosParticleHandle = Primitive->BodyInstance.ActorHandle;
			SetGlobalPoseWhileLockedWithoutAccelerationUpdate(ChaosParticleHandle, InTransform, AccelerationQueue);
		}
		
		for (USceneComponent* Component : InComp->GetAttachChildren())
		{
			// Recursive
			SetPhysicsBodyTransformBatchedWhilePhysicsLocked(Component, Component->GetComponentToWorld(), AccelerationQueue);
		}
	};
	

		// Thanks to vblanco for this fast transform setting trick
	// According to him the first two steps (comp to world and updatebounds) can be threaded but not the render dirty

		static void SetWorldTransformFastPathParallelStep(USceneComponent* InComp, const FTransform& InTransform)
		{
			
			// directly set transform and update bounds 
			InComp->SetComponentToWorld(InTransform);
			InComp->UpdateBounds();

			for (USceneComponent* Component : InComp->GetAttachChildren())
			{
				// This * transforms from local space to world space!
				FTransform CompWorldTransform = Component->GetRelativeTransform() * InTransform;

				//These are to support non-relative transforms (could probably omit as this is rare?)

				if (UNLIKELY(InComp->IsUsingAbsoluteLocation()))
				{
					CompWorldTransform.CopyTranslation(InTransform);
				}

				if (UNLIKELY(InComp->IsUsingAbsoluteRotation()))
				{
					CompWorldTransform.CopyRotation(InTransform);
				}

				if (UNLIKELY(InComp->IsUsingAbsoluteScale()))
				{
					CompWorldTransform.CopyScale3D(InTransform);
				}

				// Recursive!
				SetWorldTransformFastPathParallelStep(Component, CompWorldTransform);
			}
		};

		static void SetWorldTransformFastPathMainThreadStep(USceneComponent* InComp, const FTransform& InTransform)
		{
			// Dirty the render transform.
			// What this actually does is add these to the array of components that must be updated at the end of the game frame
			InComp->MarkRenderTransformDirty();

			for (USceneComponent* Component : InComp->GetAttachChildren())
			{
				// Recursive
				SetWorldTransformFastPathMainThreadStep(Component, InTransform);
			}
		};

	// This is not the fastest way to go because it forces repeated changes to the acceleration structure that should be batched
	// The manual way it much more complicated though and I want to keep this from being intimidating to look at
	static void SetPhysicsBodyTransformWhilePhysicsLocked(USceneComponent* InComp, const FTransform& InTransform)
	{
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(InComp))
		{
			// this is NOT an AActor, chaos calls bodies actors because... reasons
			FPhysicsActorHandle ChaosParticleHandle = Primitive->BodyInstance.ActorHandle;
			FPhysicsInterface::SetGlobalPose_AssumesLocked(ChaosParticleHandle, InTransform);
		}
		
		for (USceneComponent* Component : InComp->GetAttachChildren())
		{
			// Recursive
			SetPhysicsBodyTransformWhilePhysicsLocked(Component, Component->GetComponentToWorld());
		}
	};
};
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSMassTransformToSceneCompFastPathTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSTransformToSceneCompTranslatorFastPath : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMSTransformToSceneCompTranslatorFastPath();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;




	FMassEntityQuery ParallelStepQuery;

	FMassEntityQuery MainThreadStepQuery;
};


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSSceneComponentTransformToMassTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSSceneCompTransformToMassTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMSSceneCompTransformToMassTranslator();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
