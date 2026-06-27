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
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
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



const FName NAME_MassSampleChaosUserDefinedDataEntityTypeName = TEXT("MassSampleChaosUserDefinedDataEntity");

FMassSampleBodyInstanceOwner::FMassSampleBodyInstanceOwner() : FChaosUserDefinedEntity(NAME_MassSampleChaosUserDefinedDataEntityTypeName)
{
}


bool FMassSampleBodyInstanceOwner::IsStaticPhysics() const
{
	return false;
}

UObject* FMassSampleBodyInstanceOwner::GetSourceObject() const
{
	auto Owner = OwnerSubsystem.Get();
	check(Owner);
	return Owner;
}



ECollisionResponse FMassSampleBodyInstanceOwner::GetCollisionResponseToChannel(ECollisionChannel Channel) const
{
	return BodyInstance.GetResponseToChannel(Channel);
}

bool FMassSampleBodyInstanceOwner::IsMultiBodyOverlap() const
{
	// I assume we don't want multiple body overlaps for now
	return false;
}

UObject* FMassSampleBodyInstanceOwner::GetSourceObjectOwner() const
{
	return OwnerSubsystem.Get();
}

FTransform FMassSampleBodyInstanceOwner::GetPhysicsOwnerTransform() const
{
	// Does this refer to the body instance or the Mass entity transform? I am unsure
	return BodyInstance.GetUnrealWorldTransform();
}

FTransform FMassSampleBodyInstanceOwner::GetPhysicsOwnerSocketTransform(FName InSocketName) const
{
	ensureMsgf(false, TEXT("FMassSampleBodyInstanceOwner::GetPhysicsOwnerSocketTransform doesn't do anything yet"));
	return FTransform::Identity;
}

ECollisionChannel FMassSampleBodyInstanceOwner::GetCollisionObjectType() const
{
	return BodyInstance.GetObjectType();
}

ECollisionEnabled::Type FMassSampleBodyInstanceOwner::GetCollisionEnabled() const
{
	// @todo we should store this state directly
	return BodyInstance.GetCollisionEnabled();
}

UBodySetup* FMassSampleBodyInstanceOwner::GetPhysicsBodySetup() const
{
	return BodyInstance.GetBodySetup();
}

const FWalkableSlopeOverride& FMassSampleBodyInstanceOwner::GetWalkableSlopeOverride() const
{
	return BodyInstance.GetWalkableSlopeOverride();
}

Chaos::FPhysicsObject* FMassSampleBodyInstanceOwner::GetPhysicsObjectById(Chaos::FPhysicsObjectId Id) const
{
	if (!BodyInstance.IsValidBodyInstance())
	{
		return nullptr;
	}

	return BodyInstance.GetPhysicsActor()->GetPhysicsObject();
}

bool FMassSampleBodyInstanceOwner::IsPhysicsOwnerMovable() const
{
	ensureMsgf(false, TEXT("FMassSampleBodyInstanceOwner::IsPhysicsOwnerMovable doesn't do anything yet"));

	//@todo add mobility settings
	return true;
}

bool FMassSampleBodyInstanceOwner::IsPhysicsOwnerSimulatingPhysics() const
{
	ensureMsgf(false, TEXT("FMassSampleBodyInstanceOwner::IsPhysicsOwnerSimulatingPhysics doesn't do anything yet"));

	//@todo add simulating settings
	return true;
}

FVector FMassSampleBodyInstanceOwner::GetPhysicsOwnerVelocity() const
{
	ensureMsgf(false, TEXT("FMassSampleBodyInstanceOwner::GetPhysicsOwnerVelocity doesn't do anything yet"));

	return FVector::ZeroVector;
}

UObject* FMassSampleBodyInstanceOwner::GetPhysicsOwnerAttachmentRoot() const
{
	// This doesn't seem to be used much except for character based movement on actors... 
	// I don't think we need to return this unless we have an idea of attachment and this actually is meaningful
	// I would suggest using regular actors for things that ACharacters can stand on for now
	return nullptr;
}

bool FMassSampleBodyInstanceOwner::IsPhysicsObjectWorldGeometry() const
{
	// The engine returns Mobility != EComponentMobility::Movable and if we are not static
	// @todo if IsPhysicsOwnerMovable is done this should probably call that as well? unsure
	return GetCollisionObjectType() == ECC_WorldStatic;
}

bool FMassSampleBodyInstanceOwner::DoesSocketExistOnPhysicsOwner(FName InSocketName) const
{
	// We don't have any sockets yet
	return false;
}

TArray<Chaos::FPhysicsObject*> FMassSampleBodyInstanceOwner::GetAllPhysicsObjects() const
{
	// I am not sure if we ever want you you to have more than one. 
	// I see no reason why you should limit yourself to 1 entity = 1 body but the tooling required to make that nice is way out of the scope of this example 
	if (!BodyInstance.IsValidBodyInstance())
	{
		return {};
	}

	return {BodyInstance.GetPhysicsActor()->GetPhysicsObject()};
}

FBodyInstance* FMassSampleBodyInstanceOwner::GetBodyInstance(FName BoneName, bool bGetWelded, int32 Index) const
{
	
#if DO_ENSURE
	// A bit wacky but I want to catch this assumption if it is made just in case...
	if (bGetWelded)
	{
		ensureMsgf(BodyInstance.WeldParent == nullptr, 
			TEXT("FMassSampleBodyInstanceOwner::GetBodyInstance was called with bGetWelded and we have a welded parent but do not return it "
				"consider followoing what UPrimitiveComponent::GetBodyInstance does to return the weld parent conditionally"));
	}
#endif
	
	return const_cast<FBodyInstance*>(&BodyInstance);
}

UPhysicalMaterial* FMassSampleBodyInstanceOwner::GetPhysicsMaterialOverride() const
{
	return BodyInstance.GetPhysMaterialOverride();
}

UMaterialInterface* FMassSampleBodyInstanceOwner::GetPhysicsMaterialBase() const
{
	// I am not sure how to line up the UPhysicalMaterial to a regular material... We might have to actually phone home into Mass here
	// @todo add a pointer to the material interaface (and in GetNumMaterials, GetMaterial etc)
	return nullptr;
}

int32 FMassSampleBodyInstanceOwner::GetNumMaterials() const
{
	return 0;
}

UMaterialInterface* FMassSampleBodyInstanceOwner::GetMaterial(int32 Index) const
{
	return nullptr;
}

TWeakObjectPtr<UObject> FMassSampleBodyInstanceOwner::GetOwnerObject()
{
	check(OwnerSubsystem.IsValid());
	return OwnerSubsystem;
}

FMassEntityHandle UMassSamplePhysicsStorage::FindEntityHandleFromHitResult(const FHitResult& HitResult)
{
	if (UMassSamplePhysicsStorage* Owner = Cast<UMassSamplePhysicsStorage>(HitResult.PhysicsObjectOwner.Get()))
	{
		if (TSharedPtr<FMassSampleBodyInstanceOwner>* ObjectOwner = Owner->BodyInstanceOwners.Find(HitResult.PhysicsObject))
		{
			if (FMassSampleBodyInstanceOwner* OwnerPtr = ObjectOwner->Get())
			{
				return OwnerPtr->EntityHandle;
			}
		}
	}
	
	return FMassEntityHandle();
}

IPhysicsBodyInstanceOwner* UMassSamplePhysicsStorage::ResolvePhysicsBodyInstanceOwner(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	if (PhysicsObject)
	{
		FLockedReadPhysicsObjectExternalInterface PhysicsObjectInterface = FPhysicsObjectExternalInterface::LockRead(PhysicsObject);
		FChaosUserDefinedEntity* UserDefinedEntity = PhysicsObjectInterface->GetUserDefinedEntity(PhysicsObject);

		// chaos user data is type safe with a simple name member
		if (UserDefinedEntity && UserDefinedEntity->GetEntityTypeName() == NAME_MassSampleChaosUserDefinedDataEntityTypeName)
		{
			FMassSampleBodyInstanceOwner* FastGeoPhysicsBodyInstanceOwner = static_cast<FMassSampleBodyInstanceOwner*>(UserDefinedEntity);
			check(FastGeoPhysicsBodyInstanceOwner->GetOwnerObject().IsValid());
			return FastGeoPhysicsBodyInstanceOwner;
		}
	}

	return nullptr;
}

UMSChaosMassTranslationProcessorsProcessors::UMSChaosMassTranslationProcessorsProcessors() : ChaosSimToMass(*this), UpdateChaosKinematicTargets(*this), MassTransformsToChaosBodies(*this)
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
	FPhysScene* PhysicsScene = GetWorld()->GetPhysicsScene();
	if (!PhysicsScene)
	{
		return;
	}
	
	ChaosSimToMass.ForEachEntityChunk( Context, [&](FMassExecutionContext& Context)
	{
		auto PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if ( PhysicsHandle)
			{
				Chaos::FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();
				Transforms[i].GetMutableTransform() = FTransform(Body_External.R(), Body_External.X());;
			}
		}
	});

	// mass forces to kinematic targets
	UpdateChaosKinematicTargets.ForEachEntityChunk( Context, [&](FMassExecutionContext& Context)
	{
		const auto& PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		const auto& Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const auto& Forces = Context.GetFragmentView<FMassForceFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (PhysicsHandle)
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
	
	MassTransformsToChaosBodies.ForEachEntityChunk( Context, [&](FMassExecutionContext& Context)
	{
		// This one is by value as we do an evil reinterpret cast later?
		TConstArrayView<FMSMassPhysicsFragment> PhysicsFragments = Context.GetFragmentView<FMSMassPhysicsFragment>();
		const auto& Transforms = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); i++)
		{
			// mostly a dupe of FChaosEngineInterface::SetGlobalPose_AssumesLocked with some small changes
			FPhysicsActorHandle PhysicsHandle = PhysicsFragments[i].SingleParticlePhysicsProxy;
			if (PhysicsHandle)
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
				if (PhysicsHandle)
				{
					// I am not super confident in this part. I think I am almost certainly missing a niceer way to do this.
					using namespace Chaos;
					
					FRigidBodyHandle_External& Body_External = PhysicsHandle->GetGameThreadAPI();
					FRigidTransform3 RigidTransform(Body_External.X(), Body_External.R());

					if(Body_External.ShapesArray().Num() == 1)
					{
						
						Chaos::DebugDraw::DrawShape(RigidTransform, Body_External.GetGeometry(), Body_External.ShapesArray()[0].Get(), FColor::Silver, 0.0f, &ChaosMassPhysDebugDebugDrawSettings);

					}
					else if(Body_External.ShapesArray().Num() > 1)
					{
					
						for (const TUniquePtr<FPerShapeData>& PerShapeData : Body_External.ShapesArray()) 
						{
							Chaos::DebugDraw::DrawShape(RigidTransform, Body_External.GetGeometry(), PerShapeData.Get(), FColor::Silver, 0.0f, &ChaosMassPhysDebugDebugDrawSettings);
						}
					}
				}
			}
#endif
			
		}
		static_assert(std::is_convertible_v<FPhysicsActorHandle, FMSMassPhysicsFragment>,
		              "We want to convert from FMSMassPhysicsFragment to FMSMassPhysicsFragment here to avoid making new array or this reinterpret_cast is going to get weird");
		

		// Update all of of this chunk at once;
		GetWorld()->GetPhysicsScene()->UpdateActorsInAccelerationStructure(reinterpret_cast<TArray<FPhysicsActorHandle>&>(PhysicsFragments));
	});
}
