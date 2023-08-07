// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMassPhysics.h"

#include "MSMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassSimulationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

UMSChaosMassTranslationProcessorsProcessors::UMSChaosMassTranslationProcessorsProcessors()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	// Unfortunately it's not so simple to change stuff in chaos from this side yet, I need to research some more to get a real 
	bRequiresGameThreadExecution = true;
}

void UMSChaosMassTranslationProcessorsProcessors::ConfigureQueries()
{
	// using our fancy MSMassUtils template here
	ChaosSimToMass = MSMassUtils::Query<FMSChaosToMassTag, const FMSMassPhysicsFragment, FTransformFragment>();
	ChaosSimToMass.RegisterWithProcessor(*this);
	
	MassTransformsToChaosBodies = MSMassUtils::Query<FMSMassToChaosTag, FMSMassPhysicsFragment, const FTransformFragment>();
	MassTransformsToChaosBodies.RegisterWithProcessor(*this);

	UpdateChaosKinematicTargets = MassTransformsToChaosBodies;
	UpdateChaosKinematicTargets.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
	UpdateChaosKinematicTargets.RegisterWithProcessor(*this);
}

void UMSChaosMassTranslationProcessorsProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ChaosSimToMass.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
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
	UpdateChaosKinematicTargets.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
				if (Body_External.UpdateKinematicFromSimulation())
				{
					Body_External.SetKinematicTarget(NewPose + FTransform(Forces[i].Value));
				}
#endif
				
			}
		}
	});
	MassTransformsToChaosBodies.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
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
		}
		static_assert(std::is_convertible_v<FPhysicsActorHandle, FMSMassPhysicsFragment>,
		              "We want to convert from FMSMassPhysicsFragment to FMSMassPhysicsFragment here to avoid making new array or this reinterpret_cast is going to get weird")
			;
		// Update all of of this chunk at once;
		GetWorld()->GetPhysicsScene()->UpdateActorsInAccelerationStructure(reinterpret_cast<TArray<FPhysicsActorHandle>&>(PhysicsFragments));
	});
}
