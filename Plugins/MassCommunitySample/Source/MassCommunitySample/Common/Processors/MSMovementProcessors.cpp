// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMovementProcessors.h"

#include "MassExecutionContext.h"
#include "MassCommon/Public/MassCommonFragments.h"
#include "MassMovement/Public/MassMovementFragments.h"
#include "Common/Fragments/MSFragments.h"


UMSGravityProcessor::UMSGravityProcessor()
{
	//This executes on any type of game client (server, standalone, client etc).
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	//You can combine flags if you want:
	//ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);

	//Using the built-in movement processor group, we want to execute before it does!
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMSGravityProcessor::ConfigureQueries()
{
	// Only include entities that meet the following rules:
	
		// ALL must have an FMoverTag (just to split this up from other similar queries)
		GravityEntityQuery.AddTagRequirement<FMSGravityTag>(EMassFragmentPresence::All);
		// must have an FMassForceFragment and we are reading and mutating it 
		GravityEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

		// Finally,  we make the register aware of our new query as we define it ourselves
		GravityEntityQuery.RegisterWithProcessor(*this);
}

void UMSGravityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	//The processor's work begins once a frame (as a regular processor) in tickinggroup PrePhysics by default
	
	// Here we store a useful value we know won't change during entity iteration
	float GravityZ = GetWorld()->GetGravityZ();
	

	//Note that this is a lambda! If you want extra data you may need to pass something into the [] (or just be lazy and use &)
	GravityEntityQuery.ForEachEntityChunk(EntityManager, Context, [GravityZ](FMassExecutionContext& Context)
	{
		//Get the length of the entities in our current ExecutionContext
		const int32 NumEntities = Context.GetNumEntities();
		
		//These are what let us read and change entity data from the query in the ForEach
		TArrayView<FMassVelocityFragment> TransformList = Context.GetMutableFragmentView<FMassVelocityFragment>();
		
		//Loop over every entity in the current chunk and do stuff!
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FVector& Velocity = TransformList[EntityIndex].Value;

			Velocity +=  FVector(0,0,GravityZ) * Context.GetDeltaTimeSeconds();
			
		}
	});
}


UMSBasicMovementProcessor::UMSBasicMovementProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UMSBasicMovementProcessor::ConfigureQueries()
{
	MovementEntityQuery.AddTagRequirement<FMSBasicMovement>(EMassFragmentPresence::All);
	MovementEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	//must have an FMassForceFragment and we are only reading it
	MovementEntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
	MovementEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	MovementEntityQuery.RegisterWithProcessor(*this);

	// A simple query that forces the transform to rotate in the direction of velocity 
	RotationFollowsVelocity.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	RotationFollowsVelocity.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	RotationFollowsVelocity.AddTagRequirement<FMSRotationFollowsVelocityTag>(EMassFragmentPresence::All);
	RotationFollowsVelocity.RegisterWithProcessor(*this);
}

void UMSBasicMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	MovementEntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_MovementEntityQuery);

		const int32 NumEntities = Context.GetNumEntities();
		
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		
		//This one is readonly, so we don't need Mutable
		const TConstArrayView<FMassForceFragment> ForceList = Context.GetFragmentView<FMassForceFragment>();
		
		const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

		float DeltaTime = Context.GetDeltaTimeSeconds();

		//Loop over every entity in the current chunk and do stuff!
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransform& TransformToChange = TransformList[EntityIndex].GetMutableTransform();

			const FVector& Force = ForceList[EntityIndex].Value;

			
			FVector& Velocity = VelocityList[EntityIndex].Value;

			// Force steers the velocity 
			Velocity += Force * DeltaTime;
			
			// Move the actual transform
			TransformToChange.AddToTranslation(Velocity * DeltaTime);
		}
	});

	// This is a second query that is ran right after the first
	RotationFollowsVelocity.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ExecContext)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_RotationFollowsVelocity);

		const auto Velocities = ExecContext.GetFragmentView<FMassVelocityFragment>();
		auto Transforms = ExecContext.GetMutableFragmentView<FTransformFragment>();

		int32 NumEntities = ExecContext.GetNumEntities();

		FCollisionQueryParams QueryParams;

		for (int32 i = 0; i < NumEntities; ++i)
		{
			auto& Transform = Transforms[i].GetMutableTransform();

			Transform.SetRotation(Velocities[i].Value.ToOrientationQuat());
		}
	});
	
}
