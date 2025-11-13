// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPhysicsInitProcessors.h"

#include "MSMassPhysicsTypes.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MSMassCollision.h"
#include "MSMassPhysics.h"
#include "Experimental/LambdaBasedMassProcessor.h"
#include "Physics/PhysicsFiltering.h"
#include "Physics/Experimental/ChaosInterfaceUtils.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsBodyInstanceOwnerInterface.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSPhysicsInitProcessors)


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
// Currently unused as it's just vastly easier to store a body instance somewhere and use that...
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


FPhysicsActorHandle MassSampleCreatePhysicsStateFromInstanceOwner(FPhysScene* PhysScene, UBodySetup* BodySetup,
                                                                  FMassSampleBodyInstanceOwner& BodyInstanceOwner, const FTransform& WorldTransform,
                                                                  FInitBodySpawnParams& InitBodySpawnParams)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MassSampleCreatePhysicsState);
	FBodyInstance& BodyInstance = BodyInstanceOwner.BodyInstance;

	// This should not be valid already
	if (BodyInstance.IsValidBodyInstance())
	{
		return nullptr;
	}

	if (BodySetup)
	{
		FTransform BodyTransform = WorldTransform;

		// Here we make sure we don't have zero scale. This still results in a body being made and placed in
		// world (very small) but is consistent with a body scaled to zero.
		const FVector BodyScale = BodyTransform.GetScale3D();
		if (BodyScale.IsNearlyZero())
		{
			BodyTransform.SetScale3D(FVector(UE_KINDA_SMALL_NUMBER));
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if ((BodyInstance.GetCollisionEnabled() != ECollisionEnabled::NoCollision) && (FMath::IsNearlyZero(BodyScale.X) ||
			FMath::IsNearlyZero(BodyScale.Y) || FMath::IsNearlyZero(BodyScale.Z)))
		{
			// UE_LOG(LogFastGeoStreaming, Warning, TEXT("Scale for FastGeoPrimitiveComponent has a component set to zero, which will result in a bad body instance. Scale:%s"), *BodyScale.ToString());

			// User warning has been output - fix up the scale to be valid for physics
			BodyTransform.SetScale3D(FVector(FMath::IsNearlyZero(BodyScale.X) ? UE_KINDA_SMALL_NUMBER : BodyScale.X,
			                                 FMath::IsNearlyZero(BodyScale.Y) ? UE_KINDA_SMALL_NUMBER : BodyScale.Y,
			                                 FMath::IsNearlyZero(BodyScale.Z) ? UE_KINDA_SMALL_NUMBER : BodyScale.Z));
		}
#endif

		// Initialize the body instance
		BodyInstance.InitBody(BodySetup, BodyTransform, nullptr, PhysScene, InitBodySpawnParams, &BodyInstanceOwner);
		

		// Assign BodyInstanceOwner
		if (FPhysicsActorHandle ProxyHandle = BodyInstance.GetPhysicsActor())
		{
			if (Chaos::FPhysicsObject* PhysicsObject = BodyInstance.IsValidBodyInstance() ? BodyInstance.GetPhysicsActor()->GetPhysicsObject() : nullptr)
			{
				TArrayView<Chaos::FPhysicsObject*> PhysicsObjects(&PhysicsObject, 1);
				FPhysicsObjectExternalInterface::LockWrite(PhysicsObjects)->SetUserDefinedEntity(PhysicsObjects, &BodyInstanceOwner);

				return ProxyHandle;
			}
		}
	}

	return nullptr;
}



UMSPhysicsInitProcessor::UMSPhysicsInitProcessor() : EntityQuery(*this)
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
	// PHyisics API stuff is picky here
	bRequiresGameThreadExecution = true;
}

void UMSPhysicsInitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMSMassPhysicsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassSamplePhysicsStorage>(EMassFragmentAccess::ReadWrite);

	// try FAggGeomFragment first and then  FMSSharedStaticMesh's static mesh's geom
	EntityQuery.AddSharedRequirement<FSharedCollisionSettingsFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	// this is const as it's just an asset ptr
	EntityQuery.AddConstSharedRequirement<FMSSharedStaticMesh>(EMassFragmentPresence::Optional);


	EntityQuery.RegisterWithProcessor(*this);
}

void UMSPhysicsInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{

	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

	// We use a static mesh asset's collision geo to init a new physics body 
	EntityQuery.ForEachEntityChunk(InContext,
		[this,&PhysScene](FMassExecutionContext& Context)
		{
			auto PhysicsFragments = Context.GetMutableFragmentView<FMSMassPhysicsFragment>();
			auto Transforms = Context.GetFragmentView<FTransformFragment>();

			auto& PhysicsStorage = Context.GetMutableSubsystemChecked<UMassSamplePhysicsStorage>();


			FBodyInstance* BodySetupBodyInstance = nullptr;


			bool bShouldSimulatePhysics = false;


			if (Context.DoesArchetypeHaveTag<FMSSimulatesPhysicsTag>())
			{
				bShouldSimulatePhysics = true;
			}

			UBodySetup* BodySetup = nullptr;
			if (const FMSSharedStaticMesh* SharedStaticMesh = Context.GetConstSharedFragmentPtr<FMSSharedStaticMesh>())
			{
				if (!ensure(SharedStaticMesh->StaticMesh))
				{
					return;
				}

				// @fixme a nasty LoadSynchronous that should be avoided...
				UStaticMesh* StaticMesh = SharedStaticMesh->StaticMesh.LoadSynchronous();
				BodySetup = StaticMesh->GetBodySetup();
				BodySetupBodyInstance = &BodySetup->DefaultInstance;
			};


			// We want to try a dedicated collision fragment before using a static mesh 
			if (FSharedCollisionSettingsFragment* SharedCollisionSettings = Context.GetMutableSharedFragmentPtr<
				FSharedCollisionSettingsFragment>())
			{
				BodySetupBodyInstance = &SharedCollisionSettings->BodyInstance;
			}


			if (!BodySetupBodyInstance)
			{
				return;
			}

			for (int32 i = 0; i < Context.GetNumEntities(); i++)
			{
				const FTransform& Transform = Transforms[i].GetTransform();

				FMassEntityHandle Entity = Context.GetEntity(i);

				FMassSampleBodyInstanceOwner* NewOwnerPtr = new FMassSampleBodyInstanceOwner();
				NewOwnerPtr->OwnerSubsystem = &PhysicsStorage;
				NewOwnerPtr->EntityHandle = Entity;
				NewOwnerPtr->BodyInstance.CopyBodyInstancePropertiesFrom(BodySetupBodyInstance);
				NewOwnerPtr->BodyInstance.InstanceBodyIndex = INDEX_NONE;
				NewOwnerPtr->BodyInstance.bAutoWeld = false;
				NewOwnerPtr->BodyInstance.bSimulatePhysics = bShouldSimulatePhysics;


				FInitBodySpawnParams InitBodyParams = FInitBodySpawnParams(false, /*bPhysicsTypeDeterminesSimulation*/false);

				FPhysicsActorHandle NewActorHandle = MassSampleCreatePhysicsStateFromInstanceOwner(
					PhysScene,
					BodySetup,
					*NewOwnerPtr,
					Transform,
					InitBodyParams);


				TSharedRef<FMassSampleBodyInstanceOwner> AsShared = MakeShareable(NewOwnerPtr);
				
				if (ensure(NewActorHandle))
				{
					// Store this as a shared pointer on physics storage. This MUST be a stable pointer so there is no way this will work stored directly in archetype chunks
					// A bit of indirection here is fine here as physics interactions are often random by nature anyways
					PhysicsStorage.BodyInstanceOwners.Add(NewActorHandle->GetPhysicsObject()) = AsShared;
					PhysicsFragments[i].SingleParticlePhysicsProxy = NewActorHandle;
				}
			}
		});
}


UMSPhysicsCleanupProcessor::UMSPhysicsCleanupProcessor() : EntityQuery(*this)
{
	ObservedType = FMSMassPhysicsFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;
	// Phyisics API stuff is picky here
	bRequiresGameThreadExecution = true;
}

void UMSPhysicsCleanupProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UMassSamplePhysicsStorage>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FMSMassPhysicsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMSPhysicsCleanupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& InContext)
{
	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

	EntityQuery.ForEachEntityChunk(InContext,
       [this,&PhysScene](FMassExecutionContext& Context)
       {
           if (!ensure(PhysScene))
           {
               return;
           }
           auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();

           UMassSamplePhysicsStorage& PhysicsStorage = Context.GetMutableSubsystemChecked<UMassSamplePhysicsStorage>();

           for (int32 i = 0; i < Context.GetNumEntities(); i++)
           {
	           FMassEntityHandle Entity = Context.GetEntity(i);
           	
	           if (FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy)
	           {
	           	  PhysicsStorage.BodyInstanceOwners.Remove(PhysicsHandle->GetPhysicsObject());
		          FChaosEngineInterface::ReleaseActor(PhysicsHandle, PhysScene);
	           }
           }
       });
}
