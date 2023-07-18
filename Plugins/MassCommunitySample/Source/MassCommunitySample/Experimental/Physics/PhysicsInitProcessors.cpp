// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsInitProcessors.h"

#include "BMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MSMassCollision.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "Physics/PhysicsFiltering.h"
#include "Physics/Experimental/ChaosInterfaceUtils.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"


FBodyCollisionFilterData CollisionFilterDataDummy(const FCollisionResponseContainer& ResponseContainer)
{
	FBodyCollisionFilterData FilterData;
	// collsion enabled
	if (true)
	{
		FCollisionFilterData QueryFilterData;
		FCollisionFilterData SimFilterData;
		CreateShapeFilterData(ECC_WorldDynamic, FMaskFilter(0), 0, ResponseContainer, 0, 0, QueryFilterData, SimFilterData, true, true, true);

		// Build filterdata variations for complex and simple

		FilterData.SimFilter = SimFilterData;
		FilterData.QuerySimpleFilter = QueryFilterData;
		FilterData.QueryComplexFilter = QueryFilterData;
	}
	return FilterData;
}


FPhysicsActorHandle InitAndAddNewChaosBody(FActorCreationParams& ActorParams, const FCollisionResponseContainer& ResponseContainer,
                                           FKAggregateGeom* AggregateGeom, FPhysScene* PhysScene)
{
	check(PhysScene)
	//ActorParams.InitialTM = Transform;
	ActorParams.bStartAwake = true;
#if USE_BODYINSTANCE_DEBUG_NAMES
	ActorParams.DebugName = static_cast<char*>("Poggers");
#endif
	ActorParams.bEnableGravity = true;

#if ENGINE_MINOR_VERSION >= 3
	// Not sure about this one... I think false is the default?
	ActorParams.bUpdateKinematicFromSimulation = false;
#endif 
	ActorParams.bQueryOnly = false;
	ActorParams.bSimulatePhysics = true;
	ActorParams.Scene = PhysScene;


	FPhysicsActorHandle NewPhysHandle;

	// if (bStatic)
	// {
	// 	ActorParams.bStatic = true;
	//
	// 	FPhysicsInterface::CreateActor(ActorParams, PhysHandle);
	// }
	// else
	// {
	FPhysicsInterface::CreateActor(ActorParams, NewPhysHandle);

	FPhysicsInterface::SetCcdEnabled_AssumesLocked(NewPhysHandle, false);
	FPhysicsInterface::SetIsKinematic_AssumesLocked(NewPhysHandle, false);
	FPhysicsInterface::SetMaxLinearVelocity_AssumesLocked(NewPhysHandle, MAX_flt);
	// ???
	FPhysicsInterface::SetSmoothEdgeCollisionsEnabled_AssumesLocked(NewPhysHandle, false);
	// this seems to help make it not wiggle on the ground
	FPhysicsInterface::SetInertiaConditioningEnabled_AssumesLocked(NewPhysHandle, true);

	// Set sleep event notification?
	FPhysicsInterface::SetSendsSleepNotifies_AssumesLocked(NewPhysHandle, true);


	FGeometryAddParams CreateGeometryParams;

	CreateGeometryParams.CollisionData.CollisionFilterData = CollisionFilterDataDummy(ResponseContainer);

	CreateGeometryParams.bDoubleSided = false;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableQueryCollision = true;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableSimCollisionComplex = false;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableSimCollisionSimple = true;
	CreateGeometryParams.CollisionTraceType = ECollisionTraceFlag::CTF_UseDefault;
	CreateGeometryParams.Scale = FVector::One();
	CreateGeometryParams.LocalTransform = Chaos::FRigidTransform3::Identity;
	// No clue if this needs to be the same as the params transform
	CreateGeometryParams.WorldTransform = Chaos::FRigidTransform3::Identity;
	CreateGeometryParams.Geometry = AggregateGeom;
	// seem pointless for regular meshes? is it for skeletons?
	//CreateGeometryParams.ChaosTriMeshes = MakeArrayView(CylinderMeshBody->ChaosTriMeshes);

	// helper that makes our massive geom list
	TArray<TUniquePtr<Chaos::FImplicitObject>> Geoms;
	Chaos::FShapesArray Shapes;
	ChaosInterface::CreateGeometry(CreateGeometryParams, Geoms, Shapes);


	// use GetGameThreadAPI to... actually set the data??
	// can we not just use FPhysicsInterface::SetGeometry??
	Chaos::FRigidBodyHandle_External& Body_External = NewPhysHandle->GetGameThreadAPI();
	// geo before shapes I think... Do we want the first one only?
	Body_External.SetGeometry(MoveTemp(Geoms[0]));
	Body_External.SetShapesArray(MoveTemp(Shapes));


	TArray<FPhysicsActorHandle> Actors;
	Actors.Add(NewPhysHandle);

	FPhysicsCommand::ExecuteWrite(PhysScene, [&]()
	{
		PhysScene->AddActorsToScene_AssumesLocked(Actors, true);
	});


	//UObject owner here?
	//PhysScene->AddToComponentMaps(this, PhysHandle);

	if (true)
	{
		//PhysScene->RegisterForCollisionEvents(this);
	}

	return NewPhysHandle;
}

UMSPhysicsInitProcessors::UMSPhysicsInitProcessors()
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	bRequiresGameThreadExecution = true;
}

void UMSPhysicsInitProcessors::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMSMassPhysicsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMSSharedStaticMesh>();

	EntityQuery.RegisterWithProcessor(*this);
}

void UMSPhysicsInitProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{
	EntityQuery.ForEachEntityChunk(EntityManager, InContext, [this](FMassExecutionContext& Context)
	{
		
		auto PhysicsFragments = Context.GetMutableFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetFragmentView<FTransformFragment>();
		UStaticMesh* StaticMesh = Context.GetConstSharedFragment<FMSSharedStaticMesh>().StaticMesh.Get();

		const int32 Num = Context.GetNumEntities();
		for (int32 i = 0; i < Num; i++)
		{
			UBodySetup* BodySetup = StaticMesh->GetBodySetup();
			FKAggregateGeom* AggregateGeom = &BodySetup->AggGeom;
			if(!ensure(BodySetup) || !ensure(AggregateGeom->GetElementCount() > 0))
			{
				continue;
			}
			FCollisionResponseContainer ResponseContainer = BodySetup->DefaultInstance.GetResponseToChannels();

			FActorCreationParams ActorParams;
			ActorParams.InitialTM = Transforms[i].GetTransform();
			FPhysicsActorHandle NewHandle = InitAndAddNewChaosBody(ActorParams, ResponseContainer, AggregateGeom, GetWorld()->GetPhysicsScene());


			PhysicsFragments[i].SingleParticlePhysicsProxy = NewHandle;
		}
	});
}
