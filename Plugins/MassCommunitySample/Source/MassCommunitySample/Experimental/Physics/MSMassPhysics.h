// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassProcessor.h"
#include "Chaos/ChaosUserEntity.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsBodyInstanceOwnerInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSMassPhysics.generated.h"



// This class exists only to be used as user data inside of chaos to point to our body instance owner.
class FMassSampleBodyInstanceOwner : public FChaosUserDefinedEntity, public IPhysicsBodyInstanceOwner , public TSharedFromThis<FMassSampleBodyInstanceOwner>
{
public:
	FMassSampleBodyInstanceOwner();

	virtual ~FMassSampleBodyInstanceOwner() = default;


	// IPhysicsBodyInstanceOwner interface begin
	virtual bool IsStaticPhysics() const override;
	virtual UObject* GetSourceObject() const override;
	virtual ECollisionResponse GetCollisionResponseToChannel(ECollisionChannel Channel) const override;
	
	virtual bool IsMultiBodyOverlap() const override;
	virtual UObject* GetSourceObjectOwner() const override;
	virtual FTransform GetPhysicsOwnerTransform() const override;
	virtual FTransform GetPhysicsOwnerSocketTransform(FName InSocketName) const override;
	virtual ECollisionChannel GetCollisionObjectType() const override;
	virtual ECollisionEnabled::Type GetCollisionEnabled() const override;
	virtual UBodySetup* GetPhysicsBodySetup() const override;
	virtual const FWalkableSlopeOverride& GetWalkableSlopeOverride() const override;
	virtual Chaos::FPhysicsObject* GetPhysicsObjectById(Chaos::FPhysicsObjectId Id) const override;
	virtual bool IsPhysicsOwnerMovable() const override;
	virtual bool IsPhysicsOwnerSimulatingPhysics() const override;
	virtual FVector GetPhysicsOwnerVelocity() const override;
	virtual UObject* GetPhysicsOwnerAttachmentRoot() const override;
	virtual bool IsPhysicsObjectWorldGeometry() const override;
	virtual bool DoesSocketExistOnPhysicsOwner(FName InSocketName) const override;
	virtual TArray<Chaos::FPhysicsObject*> GetAllPhysicsObjects() const override;
	virtual FBodyInstance* GetBodyInstance(FName BoneName = NAME_None, bool bGetWelded = true, int32 Index = INDEX_NONE) const override;
	virtual UPhysicalMaterial* GetPhysicsMaterialOverride() const override;
	virtual UMaterialInterface* GetPhysicsMaterialBase() const override;
	virtual int32 GetNumMaterials() const override;
	virtual UMaterialInterface* GetMaterial(int32 Index) const override;
	//IPhysicsBodyInstanceOwner interface end
	
	// FChaosUserDefinedEntity:
	virtual TWeakObjectPtr<UObject> GetOwnerObject() override;

	
	// The static mesh used for this body instance
	TWeakObjectPtr<UStaticMesh> StaticMesh = nullptr;

	// The internal body instance used for this entity
	FBodyInstance BodyInstance = FBodyInstance();

	// The entity handle this body instance is associated with
	FMassEntityHandle EntityHandle = FMassEntityHandle();

	// The owning physics storage. This is how we resolve from this pointer to the storage
	TWeakObjectPtr<class UMassSamplePhysicsStorage> OwnerSubsystem = nullptr;
};

UCLASS()
class UMassSamplePhysicsStorage : public UWorldSubsystem, public IPhysicsBodyInstanceOwnerResolver
{
	GENERATED_BODY()

public:

	static FMassEntityHandle FindEntityHandleFromHitResult(const FHitResult& HitResult); 
	virtual IPhysicsBodyInstanceOwner* ResolvePhysicsBodyInstanceOwner(Chaos::FConstPhysicsObjectHandle PhysicsObject) override;

	TMap<Chaos::FPhysicsObjectHandle, TSharedPtr<FMassSampleBodyInstanceOwner>> BodyInstanceOwners;
};

/**
 * 
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSChaosMassTranslationProcessorsProcessors : public UMassProcessor
{
	GENERATED_BODY()
	UMSChaosMassTranslationProcessorsProcessors();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery ChaosSimToMass;

	FMassEntityQuery UpdateChaosKinematicTargets;
	FMassEntityQuery MassTransformsToChaosBodies;


};
