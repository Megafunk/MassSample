// Fill out your copyright notice in the Description page of Project Settings.


#include "MSSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassReplicationTypes.h"
#include "Common/Fragments/MSFragments.h"
#include "Example/MassVelocityRandomizerTrait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSSubsystem)

void UMSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	float OctreeSize = GetDefault<UMassSampleSettings>()->OctreeBoundsSize;
	MassSampleOctree2 = FMSOctree2(FVector::Zero(),OctreeSize);
}

bool UMSSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const {
	// Only use game worlds for now
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}


int32 UMSSubsystem::SampleSpawnEntityExamples()
{
	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	/** NOTE: It should be noted that this is a very "raw" way to build entities,
	* it serves to show how they can be manipulated with direct calls to the mass entity manager
	* In most cases you will be changing entities by deferred commands and processors, rather than raw calls like this
	*/ 
	
	//To spawn entities raw from C++ we can make a new archetype like so:
		FMassArchetypeHandle MoverArchetype =  EntityManager.CreateArchetype(
	{
		FTransformFragment::StaticStruct(),
		FMassVelocityFragment::StaticStruct()
	});

	// You could cache it, but this is just an example
	
	// Create and store a handle to our new entity
	// this is a REALLY "raw" example and not how you generally use Mass (you don't need to manually change or declare archetypes like this). Mostly for demonstration
	FMassEntityHandle NewEntity = EntityManager.CreateEntity(MoverArchetype);

	// NOTE: to be clear, Mass has many other ways to change and read entity data that are more designed for user code.
	// Doing stuff raw like this is not the norm, it is mostly for demonstration
	// Generally we defer changing entity composition with deferred commands etc
	// You can see some of those further below

	
	//Afterwards you can add, remove or change fragments of the entity. Let's add a mass tag!
	EntityManager.AddTagToEntity(NewEntity,FMSGravityTag::StaticStruct());
	//Make sure you use AddTag for tags and AddFragment for fragments!
	EntityManager.AddFragmentToEntity(NewEntity,FSampleColorFragment::StaticStruct());
	
	//How about changing data on a fragment?
	EntityManager.GetFragmentDataChecked<FMassVelocityFragment>(NewEntity).Value = FMath::VRand()*100.0f;
	EntityManager.GetFragmentDataChecked<FSampleColorFragment>(NewEntity).Color = FColor::Blue;

	
	//calling mass.PrintEntityFragments 1 in editor should reveal this entity's fragments! For example:

	// Listing fragments values for Entity[i: 1 sn: 1] in /Game/ThirdPerson/Maps/ThirdPersonMap.ThirdPersonMap:MassEntitySubsystem_0
	// TransformFragment: (Transform=(Rotation=(X=0.000000,Y=0.000000,Z=0.000000,W=1.000000),Translation=(X=0.000000,Y=0.000000,Z=0.000000),Scale3D=(X=1.000000,Y=1.000000,Z=1.000000)))
	// MassVelocityFragment: ()
	// SampleColorFragment: ()
	UE_LOG( LogTemp, Warning, TEXT("Entity %i created on frame %llu"),	NewEntity.Index,GFrameCounter);
	
	// -----------------Deferement examples--------------------------------
	// Because mass is an archetype style ECS, we generally prefer to defer
	// I am mostly writing these here so I can copy them into the readme...


	// We reserve an entity here, this way the system knows not to give this index out to other processors/deferred actions etc
	FMassEntityHandle ReserverdEntity = EntityManager.ReserveEntity();

	FTransformFragment MyTransformFragment;
	MyTransformFragment.SetTransform(FTransform::Identity);

	FSampleColorFragment MyColorFragment;

	// We build a new entity and add fragments to it in one command!
	EntityManager.Defer().PushCommand<FMassCommandBuildEntity>(ReserverdEntity,MyColorFragment);


	// Flush the commands so this new entity is actually around, but not during processing
	if (!EntityManager.IsProcessing())
	{
		// This is an example because Mass will generally call FlushCommands commands for you during the frame, but sometimes immediate results are needed
		EntityManager.FlushCommands();
	}
	
	// Sets fragment data on an existing entity
	EntityManager.Defer().PushCommand<FMassCommandAddFragmentInstances>(ReserverdEntity,MyColorFragment,MyTransformFragment);
	
	// Reserve yet another entity...
	ReserverdEntity = EntityManager.ReserveEntity();

	FMSExampleSharedFragment SharedFragmentExample;
	SharedFragmentExample.SomeKindaOfData = FMath::Rand() * 10000.0f;
	FMassArchetypeSharedFragmentValues SharedFragmentValues;
	
	// This is what traits use to create their shared fragment info as well
	const FConstSharedStruct& SharedFragmentSharedStruct = EntityManager.GetOrCreateConstSharedFragment(SharedFragmentExample);
	SharedFragmentValues.Add(SharedFragmentSharedStruct);

	EntityManager.Defer().PushCommand<FMassCommandBuildEntityWithSharedFragments>(ReserverdEntity, MoveTemp(SharedFragmentValues), MyTransformFragment, MyColorFragment);
	
	return NewEntity.Index;
}

