// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMassPhysics.h"

#include "MSMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassSimulationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"


void UMSPhysicsWorldSubS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto EntitySim = Collection.InitializeDependency<UMassSimulationSubsystem>();

	// chaos transforms to mass transforms (uses dynamic MSMassUtils::GameThreadProcessor, nothing special about this processor-wise besides that)
	MSMassUtils::GameThreadProcessor<FMSChaosToMassTag, const FMSMassPhysicsFragment, FTransformFragment>(EntitySim).ForEachChunk([this](FMassExecutionContext& Context)
	{
		auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
			{
				Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();

				auto Mytransform = FTransform(Body_External.R(), Body_External.X());

				Transforms[i].GetMutableTransform() = Mytransform;
			}
		}
	});
	
	// mass transforms to chaos
	MSMassUtils::GameThreadProcessor<FMSMassToChaosTag, FMSMassPhysicsFragment,const FTransformFragment>(EntitySim).ForEachChunk([this](FMassExecutionContext& Context)
	{
		auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (GetWorld()->GetPhysicsScene() && PhysicsHandle)
			{
				FChaosEngineInterface::SetGlobalPose_AssumesLocked(PhysicsHandle,Transforms[i].GetTransform());
			}
		}
	});
}


bool UMSPhysicsWorldSubS::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::PIE || WorldType == EWorldType::Game;
}


void UMSPhysicsWorldSubS::OnWorldBeginPlay(UWorld& InWorld)
{
}

void UMSPhysicsWorldSubS::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
