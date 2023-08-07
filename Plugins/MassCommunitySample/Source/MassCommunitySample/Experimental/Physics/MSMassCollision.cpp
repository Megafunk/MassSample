// Fill out your copyright notice in the Description page of Project Settings.


#include "MSMassCollision.h"

#include "MassCommonFragments.h"
#include "Chaos/Convex.h"
#include "PhysicsEngine/BodySetup.h"
// This is for queries against FMSSharedStaticMeshs... Now that we have true mass physics this is a tad pointless
namespace MassSample::Collision
{
	// @todo-karl: fix the implict object caching here
	bool SingleRaycastEntityStaticMesh(FVector& From, FVector& To, FHitResult& OutHit, const FMSSharedStaticMesh* SharedMesh,
		FTransform WorldTransform)
	{
		FVector Delta = To - From;

		const float DeltaMag = Delta.Size();

		FVector LocalStart = WorldTransform.InverseTransformPositionNoScale(From);
		FVector LocalDelta = WorldTransform.InverseTransformVectorNoScale(Delta);


		ChaosInterface::FRaycastHit BestHit;
		BestHit.Distance = FLT_MAX;

		// convex geo only for now...
		for (FKConvexElem Shape : SharedMesh->StaticMesh.Get()->GetBodySetup()->AggGeom.ConvexElems)
		{
			//@todo-karl cache this inside the shared fragment I suppose 
			Chaos::FConvex* Implicit = Shape.GetChaosConvexMesh();
			if(!Implicit)
			{
				continue;
			}
			Chaos::FReal Distance;

			Chaos::FVec3 LocalPosition;
			Chaos::FVec3 LocalNormal;
			int32 FaceIndex;
			// The actual chaos raycast
			if (Implicit->Raycast(LocalStart, LocalDelta / DeltaMag, DeltaMag, 0, Distance, LocalPosition, LocalNormal, FaceIndex))
			{
				if (Distance < BestHit.Distance)
				{
					BestHit.Distance = Distance;
					BestHit.WorldNormal = LocalNormal;
					//will convert to world when best is chosen
					BestHit.WorldPosition = LocalPosition;
					// BestHit.Shape = Shape;
					//BestHit.Actor = Actor->GetParticle_LowLevel();
					BestHit.FaceIndex = FaceIndex;
				}
			}
		}

		if (BestHit.Distance < FLT_MAX)
		{
			OutHit.ImpactPoint = WorldTransform.TransformPositionNoScale(BestHit.WorldPosition);
			OutHit.Location = OutHit.ImpactPoint;

			OutHit.ImpactNormal = WorldTransform.TransformVectorNoScale(BestHit.WorldNormal);

			OutHit.TraceEnd = To;
			OutHit.TraceStart = From;
			
			// DrawDebugCrosshairs(Context.GetEntityManagerChecked().GetWorld(), OutHit.ImpactPoint, OutHit.Normal.Rotation(), 10.0, FColor::Magenta, false, 10);
			return true;
		}
		return false;
	}

	bool SingleRaycastEntityView(FMassEntityView& EntityView, FVector& From, FVector& To,
		FHitResult& OutHit)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_MassLineTraceCollisionQuery);

		if (auto StaticMeshFragment = EntityView.GetConstSharedFragmentDataPtr<FMSSharedStaticMesh>())
		{
			auto WorldTransform = EntityView.GetFragmentData<FTransformFragment>().GetTransform();

			if (SingleRaycastEntityStaticMesh(From, To, OutHit, StaticMeshFragment, WorldTransform))
			{
				return true;
			};
		}
		return false;
	}
}
