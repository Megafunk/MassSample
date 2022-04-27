// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassNavigationSubsystem.h"
#include "Spatial/PointHashGrid3.h"
#include "MSHashGridFragments.generated.h"

//todo: less weird place to stuff this? some types thing? oh well...
typedef UE::Geometry::TPointHashGrid3<FMassEntityHandle,Chaos::FReal> FMSHashGrid3D;

//This entity's startomg location on our 2D hashgrid this frame
USTRUCT()
struct MASSSAMPLE_API FMSGridCellStartingLocationFragment : public FMassFragment
{
	GENERATED_BODY()
	FVector Location;
};


//to indicate the entity is currently added to the hashgrid
USTRUCT()
struct MASSSAMPLE_API FMSInHashGridTag : public FMassTag{GENERATED_BODY()};