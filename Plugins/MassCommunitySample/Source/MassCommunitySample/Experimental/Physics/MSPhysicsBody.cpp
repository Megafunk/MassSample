// Fill out your copyright notice in the Description page of Project Settings.


#include "MSPhysicsBody.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MSMassCollision.h"
#include "MSMassPhysicsTypes.h"
#include "Common/Fragments/MSFragments.h"

void UMSPhysicsBody::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMSMassPhysicsFragment>();
	// do we set the position in mass or in the physics thread? Ideally we do the latter?
	// since we are already in the physics thread we copy it there? is this safe?
	// can the physics thread mess with processing?


	// While not mutually exclusive I think it's fair to split these for now
	if (bChaosToMass)
	{
		BuildContext.AddTag<FMSChaosToMassTag>();
		if(bSimulatesPhysics)
		{
			BuildContext.AddTag<FMSSimulatesPhysicsTag>();
		}
	}
	else
	{
		BuildContext.AddTag<FMSMassToChaosTag>();
	}
	if (bHasGravity)
	{
		BuildContext.AddTag<FMSGravityTag>();
	}

	BuildContext.RequireFragment<FTransformFragment>();



	if (bManualCollisionSettingsAndGeo)
	{
		FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

		FSharedCollisionSettingsFragment CollisionSettingsFragment = CollisionSettings;

		FKSphylElem Capsule = FKSphylElem(50.0f, 400.0f);
		CollisionSettingsFragment.Geometry.SphylElems.Add(Capsule);

		uint32 Hash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(CollisionSettingsFragment));
		BuildContext.AddSharedFragment(EntityManager.GetOrCreateSharedFragmentByHash<FSharedCollisionSettingsFragment>(
			Hash, CollisionSettingsFragment));

	}
	else
	{
		BuildContext.RequireFragment<FMSSharedStaticMesh>();
	}
}
