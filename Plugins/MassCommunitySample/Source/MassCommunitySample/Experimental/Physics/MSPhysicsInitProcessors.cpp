// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPhysicsInitProcessors.h"

#include "MSMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MSMassCollision.h"
#include "Common/Fragments/MSFragments.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "Physics/PhysicsFiltering.h"
#include "Physics/Experimental/ChaosInterfaceUtils.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"


FBodyCollisionFilterData CollisionFilterDataDummy(const FCollisionResponseContainer& ResponseContainer, ECollisionTraceFlag TraceFlag)
{
	FBodyCollisionFilterData OutFilterData;

	// Yoinked from FBodyInstance::BuildBodyFilterData
	FCollisionFilterData QueryFilterData;
	FCollisionFilterData SimpleQueryData;
	CreateShapeFilterData(ECC_WorldDynamic, FMaskFilter(0), 0, ResponseContainer, 0, 0,
		QueryFilterData, SimpleQueryData, false, true, true,false);
	// Build filterdata variations for complex and simple

	FCollisionFilterData ComplexQueryData = SimpleQueryData;
			
	// Set output sim data
	OutFilterData.SimFilter = SimpleQueryData;

	// Build filterdata variations for complex and simple
	// turning this on seems to break things?
	SimpleQueryData.Word3 |= EPDF_SimpleCollision;
	if(TraceFlag == ECollisionTraceFlag::CTF_UseSimpleAsComplex)
	{
		SimpleQueryData.Word3 |= EPDF_ComplexCollision;
	}
	
	ComplexQueryData.Word3 |= EPDF_ComplexCollision;
	if(TraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		ComplexQueryData.Word3 |= EPDF_SimpleCollision;
	}
			
	OutFilterData.QuerySimpleFilter = SimpleQueryData;
	OutFilterData.QueryComplexFilter = ComplexQueryData;
	return OutFilterData;
}

// UObject free chaos body creation (mostly hardcoded as a dynamic body with no welds?).
// This does not call SetUserData as that requires a body instance so I thought I should do that elsewhere?
FPhysicsActorHandle InitAndAddNewChaosBody(FActorCreationParams& ActorParams, const FCollisionResponseContainer& ResponseContainer,
                                           FKAggregateGeom* AggregateGeom, FPhysScene* PhysScene)
{
	// Note: the term "Actor" here means chaos physics actor handle, which is not a uobject unreal actor...
	check(PhysScene)
	ActorParams.Scene = PhysScene;
	ActorParams.bStatic = false;
	ActorParams.bQueryOnly = false;
	ActorParams.bSimulatePhysics = true;

#if USE_BODYINSTANCE_DEBUG_NAMES
	ActorParams.DebugName = static_cast<char*>("MassPhysicsBody");
#endif
#if ENGINE_MINOR_VERSION >= 3
	// Not sure about this one... I think false is the default?
	ActorParams.bUpdateKinematicFromSimulation = false;
#endif
	

	
	FPhysicsActorHandle NewPhysHandle;
	// if we were static we only need this:
	// {
	// 	ActorParams.bStatic = true;
	//
	// 	FPhysicsInterface::CreateActor(ActorParams, PhysHandle);
	// }
	// else
	// {
	FPhysicsInterface::CreateActor(ActorParams, NewPhysHandle);
	
	Chaos::FRigidBodyHandle_External& Body_External = NewPhysHandle->GetGameThreadAPI();
	Body_External.SetCCDEnabled(false);
	// no clue about this one
	Body_External.SetSmoothEdgeCollisionsEnabled(false);
	// this seems to help make it not wiggle on the ground
	Body_External.SetInertiaConditioningEnabled(true);
	// I prefer to use body_external but these do a lot more than the others than set a flag
	FPhysicsInterface::SetIsKinematic_AssumesLocked(NewPhysHandle, false);
	FPhysicsInterface::SetMaxLinearVelocity_AssumesLocked(NewPhysHandle, MAX_flt);

	// currently does nothing...
	FPhysicsInterface::SetSendsSleepNotifies_AssumesLocked(NewPhysHandle, true);


	FGeometryAddParams CreateGeometryParams;
	// pass in our parameters
	CreateGeometryParams.Geometry = AggregateGeom;
	// I made this 

	// the rest are just regular generic dynamic body values so far
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableQueryCollision = true;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableSimCollisionSimple = true;
	CreateGeometryParams.CollisionTraceType = ECollisionTraceFlag::CTF_UseDefault;
	CreateGeometryParams.Scale = FVector::One();
	CreateGeometryParams.LocalTransform = Chaos::FRigidTransform3::Identity;
	// No clue if this needs to be the same as the params transform
	CreateGeometryParams.WorldTransform = Chaos::FRigidTransform3::Identity;

	CreateGeometryParams.bDoubleSided = false;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableSimCollisionComplex = false;

	CreateGeometryParams.CollisionData.CollisionFilterData = CollisionFilterDataDummy(ResponseContainer,ECollisionTraceFlag::CTF_UseDefault);


	// seem pointless for regular meshes? is it for skeletons?
	//CreateGeometryParams.ChaosTriMeshes = MakeArrayView(CylinderMeshBody->ChaosTriMeshes);

	// helper that makes our geom list
	TArray<TUniquePtr<Chaos::FImplicitObject>> Geoms;
	Chaos::FShapesArray Shapes;
	ChaosInterface::CreateGeometry(CreateGeometryParams, Geoms, Shapes);

	// geo before shapes I think... Do we want the first one only?
	Body_External.SetGeometry(MoveTemp(Geoms[0]));
	Body_External.SetShapesArray(MoveTemp(Shapes));
	return NewPhysHandle;
}

UMSPhysicsInitProcessors::UMSPhysicsInitProcessors()
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	// PHyisics API stuff is picky here
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
	// a FPhysicsActorHandle is a Chaos particle pointer (not a regular actor) 
	TArray<FPhysicsActorHandle> NewPhysicsHandles;

	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
	
	EntityQuery.ForEachEntityChunk(EntityManager, InContext, [this,&NewPhysicsHandles,&PhysScene](FMassExecutionContext& Context)
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
			//auto View = FMassEntityView(Context.GetEntityCollection().GetArchetype(),Context.GetEntity(i));
			//ActorParams.bEnableGravity = View.HasTag<FMSGravityTag>();
			ActorParams.bEnableGravity = true;
			FPhysicsActorHandle NewHandle = InitAndAddNewChaosBody(ActorParams, ResponseContainer, AggregateGeom,PhysScene);

			Chaos::FRigidBodyHandle_External& Body_External = NewHandle->GetGameThreadAPI();
			FBodyInstance& BodyInstance = StaticMesh->GetBodySetup()->DefaultInstance;

			// perhaps this should be how we can pass over hits... a manager component? 
			// BodyInstance.OwnerComponent

			// the weird indirection is because FPhysicsUserData stores an enum about what it is
			BodyInstance.PhysicsUserData = FPhysicsUserData(&BodyInstance);
			// "SetUserData" required for queries hitting this it seems due to SetHitResultFromShapeAndFaceIndex
			Body_External.SetUserData(&BodyInstance.PhysicsUserData);


			NewPhysicsHandles.Add(NewHandle);

			PhysicsFragments[i].SingleParticlePhysicsProxy = NewHandle;
		}
	});

	// todo-perf make this work on other threads or just do this all in physics callbacks
	FPhysicsCommand::ExecuteWrite(PhysScene, [&]()
	{
		// We update the solver immediately... Not sure if we should
		PhysScene->AddActorsToScene_AssumesLocked(NewPhysicsHandles, true);
	});
}
