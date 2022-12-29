// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Math/GenericOctree.h"
#include "MSOctreeFragments.generated.h"

/** An octree. */
// Just kidding, this is a simple one for Mass
// These are all template thingies required by TOctree2, so I might be doing things wrong here!
// The shared pointer for FOctreeElementId2 isn't very ~data oriented~ but it's how TOctree2 is meant to be used afaik.



struct MASSCOMMUNITYSAMPLE_API FMSEntityOctreeElement
{
	FMassEntityHandle EntityHandle;
	
	FBoxCenterAndExtent Bounds;

	TSharedPtr<FOctreeElementId2> SharedOctreeID;
};

struct FMSEntityOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	FORCEINLINE static const FBoxCenterAndExtent& GetBoundingBox(const FMSEntityOctreeElement& Element)
	{
		return Element.Bounds;
	}

	FORCEINLINE static bool AreElementsEqual(const FMSEntityOctreeElement& A, const FMSEntityOctreeElement& B)
	{
		return A.EntityHandle == B.EntityHandle;
	}

	FORCEINLINE static void SetElementId(const FMSEntityOctreeElement& Element, FOctreeElementId2 Id)
	{

		*Element.SharedOctreeID = Id;
	};
};



typedef TOctree2<FMSEntityOctreeElement, FMSEntityOctreeSemantics> FMSOctree2;


// This entity's start location on our octree this frame
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSOctreeFragment : public FMassFragment
{
	GENERATED_BODY()
	TSharedPtr<FOctreeElementId2> OctreeID;
};

// To indicate the entity is in the hashgrid
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMSInOctreeGridTag : public FMassTag
{
	GENERATED_BODY()
};