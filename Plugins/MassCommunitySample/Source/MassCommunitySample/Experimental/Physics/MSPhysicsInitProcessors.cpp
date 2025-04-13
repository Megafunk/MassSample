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
	CreateShapeFilterData(ECC_WorldDynamic, FMaskFilter(0), 0, ResponseContainer, 0, 0, QueryFilterData, SimpleQueryData, false, true, true, false);
	// Build filterdata variations for complex and simple

	FCollisionFilterData ComplexQueryData = SimpleQueryData;

	// Set output sim data
	OutFilterData.SimFilter = SimpleQueryData;

	// Build filterdata variations for complex and simple
	// turning this on seems to break things?
	SimpleQueryData.Word3 |= EPDF_SimpleCollision;
	if (TraceFlag == ECollisionTraceFlag::CTF_UseSimpleAsComplex)
	{
		SimpleQueryData.Word3 |= EPDF_ComplexCollision;
	}

	ComplexQueryData.Word3 |= EPDF_ComplexCollision;
	if (TraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		ComplexQueryData.Word3 |= EPDF_SimpleCollision;
	}

	OutFilterData.QuerySimpleFilter = SimpleQueryData;
	OutFilterData.QueryComplexFilter = ComplexQueryData;
	return OutFilterData;
}

// UObject free chaos body creation (mostly hardcoded as a dynamic body with no welds?).
// This does not call SetUserData as that requires a body instance so I thought I should do that elsewhere?
FPhysicsActorHandle InitAndAddNewChaosBody(FActorCreationParams& ActorParams, const FBodyCollisionData& CollisionData, FKAggregateGeom* AggregateGeom,
                                           float Density)
{
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

	FPhysicsInterface::SetCcdEnabled_AssumesLocked(NewPhysHandle, false);
	// we don't want the extra pruning work here as these are generally dynamic meshes...?
	FPhysicsInterface::SetSmoothEdgeCollisionsEnabled_AssumesLocked(NewPhysHandle, false);
	// this seems to help make it not wiggle on the ground
	FPhysicsInterface::SetInertiaConditioningEnabled_AssumesLocked(NewPhysHandle, true);
	// We want to set this to false if we are simulating physics
	FPhysicsInterface::SetIsKinematic_AssumesLocked(NewPhysHandle, !ActorParams.bSimulatePhysics);
	FPhysicsInterface::SetMaxLinearVelocity_AssumesLocked(NewPhysHandle, MAX_flt);

	// currently does nothing...
	FPhysicsInterface::SetSendsSleepNotifies_AssumesLocked(NewPhysHandle, true);

	//FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked()
	FGeometryAddParams CreateGeometryParams;
	// pass in our parameters

	CreateGeometryParams.Geometry = AggregateGeom;


	CreateGeometryParams.CollisionData = CollisionData;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableQueryCollision = true;
	CreateGeometryParams.CollisionData.CollisionFlags.bEnableSimCollisionSimple = true;
	CreateGeometryParams.CollisionTraceType = ECollisionTraceFlag::CTF_UseDefault;

	CreateGeometryParams.Scale = FVector::One();
	CreateGeometryParams.LocalTransform = Chaos::FRigidTransform3::Identity;
	// No clue if this needs to be the same as the params transform... I assume this is more for uh, more static stuff?
	CreateGeometryParams.WorldTransform = ActorParams.InitialTM;

	CreateGeometryParams.bDoubleSided = false;


	// seem pointless for regular meshes? is it for skeletons?
	//CreateGeometryParams.ChaosTriMeshes = MakeArrayView(CylinderMeshBody->ChaosTriMeshes);

	// helper that makes our geom list 		
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4
	TArray<TUniquePtr<Chaos::FImplicitObject>> Geoms;
#else
	TArray<Chaos::FImplicitObjectPtr> Geoms;
#endif
	Chaos::FShapesArray Shapes;
	ChaosInterface::CreateGeometry(CreateGeometryParams, Geoms, Shapes);


	// this is what most of the earlier FPhysicsInterface:: calls operate on
	Chaos::FRigidBodyHandle_External& Body_External = NewPhysHandle->GetGameThreadAPI();

	// geo before shapes
	// mimicking landscape.cpp

	
	if (Geoms.Num() > 0)
	{
		// if we already have geometry, merge it
		if (Body_External.GetGeometry())
		{
			Body_External.MergeGeometry(MoveTemp(Geoms));
		}
		else
		{
			// I'm not super clear on what Chaos expects here?
			Chaos::FImplicitObjectUnion ChaosUnionPtr = Chaos::FImplicitObjectUnion(MoveTemp(Geoms));
			Body_External.SetGeometry(ChaosUnionPtr.CopyGeometry());
		}
	}


	// Construct Shape Bounds
	for (auto& Shape : Shapes)
	{
		Chaos::FRigidTransform3 WorldTransform = Chaos::FRigidTransform3(Body_External.X(), Body_External.R());
		Shape->UpdateShapeBounds(WorldTransform);
		
	}
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 3
	Body_External.SetShapesArray(MoveTemp(Shapes));
#else
	Body_External.MergeShapesArray(MoveTemp(Shapes));
#endif

	// Calculate the mass properties based on the shapes assuming uniform density
	Chaos::FMassProperties MassProps;
	const Chaos::FShapesArray& shapesrray = Body_External.ShapesArray();

	Chaos::CalculateMassPropertiesFromShapeCollection(MassProps, shapesrray.Num(), Density, TArray<bool>(),
	                                                  [&](int32 i) { return shapesrray[i].Get(); });

	Body_External.SetCenterOfMass(MassProps.CenterOfMass);
	Body_External.SetRotationOfMass(MassProps.RotationOfMass);


	return NewPhysHandle;
}

UMSPhysicsInitProcessor::UMSPhysicsInitProcessor() : EntityQuery(*this)
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	// PHyisics API stuff is picky here
	bRequiresGameThreadExecution = true;
}

void UMSPhysicsInitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMSMassPhysicsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	// try FAggGeomFragment first and then  FMSSharedStaticMesh's static mesh's geom
	EntityQuery.AddSharedRequirement<FSharedCollisionSettingsFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	// this is const as it's just an asset ptr
	EntityQuery.AddConstSharedRequirement<FMSSharedStaticMesh>(EMassFragmentPresence::Optional);


	EntityQuery.RegisterWithProcessor(*this);
}

void UMSPhysicsInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{
	// a FPhysicsActorHandle is a Chaos particle pointer (not a regular actor) 
	TArray<FPhysicsActorHandle> NewPhysicsHandles;

	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

	// We use a static mesh asset's collision geo to init a new physics body 
	EntityQuery.ForEachEntityChunk(EntityManager, InContext, [this,&NewPhysicsHandles,&PhysScene](FMassExecutionContext& Context)
	{
		auto PhysicsFragments = Context.GetMutableFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetFragmentView<FTransformFragment>();

		FKAggregateGeom* AggregateGeom = nullptr;

		FBodyInstance* BodyInstance = nullptr;

		void* UserData = nullptr;

		float Density = 100.0f;

		// Note: the term "Actor" here means chaos physics actor handle, which is not a uobject unreal actor...
		FActorCreationParams ActorParams;

		ActorParams.bEnableGravity = Context.DoesArchetypeHaveTag<FMSGravityTag>();

		// do we care about QueryAndProbe?
		ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryOnly;
		if (Context.DoesArchetypeHaveTag<FMSSimulatesPhysicsTag>())
		{
			ActorParams.bSimulatePhysics = true;
			CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
		};
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		ActorParams.bUpdateKinematicFromSimulation = Context.DoesArchetypeHaveTag<FMSUpdateKinematicFromSimulationTag>();
#endif
		
		// We want to try a dedicated collision fragment before using a static mesh 
		if (auto* SharedCollisionSettings = Context.GetMutableSharedFragmentPtr<FSharedCollisionSettingsFragment>())
		{
			// gross but I can't figure out a nice way to avoid this
			AggregateGeom = &SharedCollisionSettings->Geometry;
			BodyInstance = &SharedCollisionSettings->BodyInstance;

			if ((AggregateGeom->GetElementCount() <= 0))
			{
				UE_LOG(LogTemp, Warning, TEXT("UMSPhysicsInitProcessors FSharedCollisionSettingsFragment has no collision geometry!"));
				return;
			}
		}
		else if (auto* SharedStaticMesh = Context.GetConstSharedFragmentPtr<FMSSharedStaticMesh>())
		{
			if (!ensure(SharedStaticMesh->StaticMesh))
			{
				return;
			}

			UStaticMesh* StaticMesh = SharedStaticMesh->StaticMesh.LoadSynchronous();
			UBodySetup* BodySetup = StaticMesh->GetBodySetup();
			BodyInstance = &BodySetup->DefaultInstance;

			// Mass... (the weight, not this ECS plugin)
			Density = BodySetup->CalculateMass();

			AggregateGeom = &BodySetup->AggGeom;
			if ((AggregateGeom->GetElementCount() <= 0))
			{
				UE_LOG(LogTemp, Warning, TEXT("UMSPhysicsInitProcessors static mesh (%s) has no collision geometry!"), *StaticMesh->GetName());
				return;
			}
		};


		if (!BodyInstance)
		{
			return;
		}

		#if USE_BODYINSTANCE_DEBUG_NAMES
				ActorParams.DebugName = (char*)*FString("MassPhysicsBody");
		#endif


		// the weird indirection is because FPhysicsUserData stores an enum about what it is with this ctor
		BodyInstance->PhysicsUserData = FPhysicsUserData(BodyInstance);
		// "SetUserData" required for queries hitting this it seems due to SetHitResultFromShapeAndFaceIndex
		UserData = &BodyInstance->PhysicsUserData;

		FBodyCollisionData CollisionData;

		// this one relies on a live non-default bodysetup + does lots of other uobject stuff so I made CollisionFilterDataDummy
		//BodyInstance->BuildBodyFilterData(CollisionData.CollisionFilterData);
		CollisionData.CollisionFilterData = CollisionFilterDataDummy(BodyInstance->GetResponseToChannels(), ECollisionTraceFlag::CTF_UseDefault);
		BodyInstance->BuildBodyCollisionFlags(CollisionData.CollisionFlags, CollisionEnabled, true /*usecomplexassimple */);

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			ActorParams.InitialTM = Transforms[i].GetTransform();
			check(PhysScene)
			ActorParams.Scene = PhysScene;
			ActorParams.bStatic = false;
			ActorParams.bQueryOnly = false;

			FPhysicsActorHandle NewHandle = InitAndAddNewChaosBody(ActorParams, CollisionData, AggregateGeom, Density);

			if(!NewHandle)
			{
				continue;
			}
			
			Chaos::FRigidBodyHandle_External& Body_External = NewHandle->GetGameThreadAPI();

			// "SetUserData" required for queries hitting this it seems due to SetHitResultFromShapeAndFaceIndex
			Body_External.SetUserData(UserData);

			NewPhysicsHandles.Add(NewHandle);

			PhysicsFragments[i].SingleParticlePhysicsProxy = NewHandle;
		}
	});

	// todo-perf make this work on other threads or just do this all in physics callbacks
	FPhysicsCommand::ExecuteWrite(PhysScene, [&]()
	{
		// We update the solver immediately... Not sure if we should
		PhysScene->AddActorsToScene_AssumesLocked(NewPhysicsHandles, false);
	});
}


UMSPhysicsCleanupProcessor::UMSPhysicsCleanupProcessor() : EntityQuery(*this)
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
	// PHyisics API stuff is picky here
	bRequiresGameThreadExecution = true;
}

void UMSPhysicsCleanupProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMSMassPhysicsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMSPhysicsCleanupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{
	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

	EntityQuery.ForEachEntityChunk(EntityManager, InContext, [this,&PhysScene](FMassExecutionContext& Context)
	{
		if (!ensure(PhysScene))
		{
			return;
		}
		auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			if (FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy)
			{
				FChaosEngineInterface::ReleaseActor(PhysicsHandle, PhysScene);
			}
		}
	});
}
