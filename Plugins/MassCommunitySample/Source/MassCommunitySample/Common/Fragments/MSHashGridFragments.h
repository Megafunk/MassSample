// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassNavigationSubsystem.h"
#include "Chaos/SpatialHash.h"
#include "Spatial/PointHashGrid3.h"
#include "MSHashGridFragments.generated.h"

// FIXMEFUNK: Less weird place to stuff this? some types thing? oh well...
typedef UE::Geometry::TPointHashGrid3<FMassEntityHandle,Chaos::FReal> FMSHashGrid3D;

// 
//typedef Chaos::TSpatialHash<Chaos::FReal> FMSSpatialHash;

// This entity's start location on our 2D hashgrid this frame
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSGridCellStartingLocationFragment : public FMassFragment
{
	GENERATED_BODY()
	FVector Location;
};

// To indicate the entity is in the hashgrid
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSInHashGridTag : public FMassTag
{
	GENERATED_BODY()
};