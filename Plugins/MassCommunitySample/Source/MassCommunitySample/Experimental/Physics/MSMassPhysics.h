// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassProcessor.h"
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
	virtual UPhysicalMaterial* GetPhysicalMaterial() const override;
	virtual void GetComplexPhysicalMaterials(TArray<UPhysicalMaterial*>& OutPhysMaterials,
														  TArray<FPhysicalMaterialMaskParams>* OutPhysMaterialMasks) const override;
	virtual ECollisionResponse GetCollisionResponseToChannel(ECollisionChannel Channel) const override;
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

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>&) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery ChaosSimToMass;

	FMassEntityQuery UpdateChaosKinematicTargets;
	FMassEntityQuery MassTransformsToChaosBodies;


};
