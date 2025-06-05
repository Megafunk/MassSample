// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMassPhysics.h"

#include "MSMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassSimulationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "Chaos/DebugDrawQueue.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSMassPhysics)

#if CHAOS_DEBUG_DRAW
Chaos::DebugDraw::FChaosDebugDrawSettings ChaosMassPhysDebugDebugDrawSettings(
	/* ArrowSize =					*/ 1.5f,
	/* BodyAxisLen =				*/ 4.0f,
	/* ContactLen =					*/ 4.0f,
	/* ContactWidth =				*/ 2.0f,
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4
	/* ContactPhiWidth =			*/ 0.0f, //this isn't around in 5.4?
#endif

	/* ContactInfoWidth				*/ 2.0f,
	/* ContactOwnerWidth =			*/ 0.0f,
	/* ConstraintAxisLen =			*/ 5.0f,
	/* JointComSize =				*/ 2.0f,
	/* LineThickness =				*/ 0.15f,
	/* DrawScale =					*/ 1.0f,
	/* FontHeight =					*/ 10.0f,
	/* FontScale =					*/ 1.5f,
	/* ShapeThicknesScale =			*/ 1.0f,
	/* PointSize =					*/ 2.0f,
	/* VelScale =					*/ 0.0f,
	/* AngVelScale =				*/ 0.0f,
	/* ImpulseScale =				*/ 0.0f,
	/* PushOutScale =				*/ 0.0f,
	/* InertiaScale =				*/ 0.0f,
	/* DrawPriority =				*/ 10.0f,
	/* bShowSimple =				*/ true,
	/* bShowComplex =				*/ false,
	/* bInShowLevelSetCollision =	*/ false,
	/* InShapesColorsPerState =     */ Chaos::DebugDraw::GetDefaultShapesColorsByState(),
	/* InShapesColorsPerShaepType=  */ Chaos::DebugDraw::GetDefaultShapesColorsByShapeType(),
	/* InBoundsColorsPerState =     */ Chaos::DebugDraw::GetDefaultBoundsColorsByState(),
	/* InBoundsColorsPerShapeType=  */ Chaos::DebugDraw::GetDefaultBoundsColorsByShapeType()
);


TAutoConsoleVariable<bool> CVMSDrawChaosBodies(
	TEXT("ms.drawchaosbodies"),
	false,
	TEXT("draw debug info for all mass chaos bodies using the chaos debug draw queue, make sure you also called p.Chaos.DebugDraw.Enabled 1"));
#endif

UMSChaosMassTranslationProcessorsProcessors::UMSChaosMassTranslationProcessorsProcessors()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	// Unfortunately it's not so simple to change stuff in chaos from this side yet, I need to research some more to get a real 
	bRequiresGameThreadExecution = true;
}

void UMSChaosMassTranslationProcessorsProcessors::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	
	// using our fancy MSMassUtils template here
	ChaosSimToMass = MSMassUtils::Query<FMSChaosToMassTag, const FMSMassPhysicsFragment, FTransformFragment>(EntityManager);
	ChaosSimToMass.RegisterWithProcessor(*this);
	
	MassTransformsToChaosBodies = MSMassUtils::Query<FMSMassToChaosTag, FMSMassPhysicsFragment, const FTransformFragment>(EntityManager);
	MassTransformsToChaosBodies.RegisterWithProcessor(*this);

	UpdateChaosKinematicTargets = MassTransformsToChaosBodies;
	UpdateChaosKinematicTargets.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
	UpdateChaosKinematicTargets.RegisterWithProcessor(*this);
}

void UMSChaosMassTranslationProcessorsProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ChaosSimToMass.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
			{
				Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();
				Transforms[i].GetMutableTransform() = FTransform(Body_External.R(), Body_External.X());;
			}
		}
	});

	// mass forces to kinematic targets
	UpdateChaosKinematicTargets.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const auto& PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		const auto& Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const auto& Forces = Context.GetFragmentView<FMassForceFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
			{
				const FTransform NewPose = Transforms[i].GetTransform();
				//FChaosEngineInterface::SetGlobalPose_AssumesLocked(PhysicsHandle,);
				Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();
				ensure(FChaosEngineInterface::IsKinematic(PhysicsHandle));
				
				// Need to clear this out if we had one before... But... what if we never do?
				// The assumption here is that ANYTHING else could set the kinematic target so
				// this sets a flag that clears out the velocity next tick in case something else comes along that wants it?
				// I feel like we should figure out if this actually matters or not
				Body_External.ClearKinematicTarget();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
				if (Body_External.UpdateKinematicFromSimulation())
				{
					Body_External.SetKinematicTarget(NewPose + FTransform(Forces[i].Value));
				}
#endif
				
			}
		}
	});
	MassTransformsToChaosBodies.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		// This one is by value as we do an evil reinterpret cast later?
		TConstArrayView<FMSMassPhysicsFragment> PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		const auto& Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			// mostly a dupe of FChaosEngineInterface::SetGlobalPose_AssumesLocked with some small changes
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
			{
				//FChaosEngineInterface::SetGlobalPose_AssumesLocked loop unrolled?

				const FTransform& NewPose = Transforms[i].GetTransform();
				//FChaosEngineInterface::SetGlobalPose_AssumesLocked(PhysicsHandle,);
				Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();

				Body_External.SetX(NewPose.GetLocation());
				Body_External.SetR(NewPose.GetRotation());
				// Always update the bounds?
				Body_External.UpdateShapeBounds();
			}

#if CHAOS_DEBUG_DRAW
			if(CVMSDrawChaosBodies.GetValueOnAnyThread())
			{
				if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
				{
					Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();

					
					Chaos::FShapeOrShapesArray ShapeUnion;
					if(Body_External.ShapesArray().Num() == 1)
					{
						ShapeUnion = Chaos::FShapeOrShapesArray(Body_External.ShapesArray()[0].Get());
					}
					else if(Body_External.ShapesArray().Num() > 1)
					{
						ShapeUnion = Chaos::FShapeOrShapesArray(&Body_External.ShapesArray());
					}

					

					Chaos::FRigidTransform3 RigidTransform(Body_External.X(), Body_External.R());
					Chaos::DebugDraw::DrawShape(RigidTransform,
						Body_External.Geometry().Get(),ShapeUnion,
						FColor::Silver,&ChaosMassPhysDebugDebugDrawSettings);

				}
				
			}
#endif
				
		}
		static_assert(std::is_convertible_v<FPhysicsActorHandle, FMSMassPhysicsFragment>,
		              "We want to convert from FMSMassPhysicsFragment to FMSMassPhysicsFragment here to avoid making new array or this reinterpret_cast is going to get weird")
			;
		// Update all of of this chunk at once;
		GetWorld()->GetPhysicsScene()->UpdateActorsInAccelerationStructure(reinterpret_cast<TArray<FPhysicsActorHandle>&>(PhysicsFragments));
	});
}
