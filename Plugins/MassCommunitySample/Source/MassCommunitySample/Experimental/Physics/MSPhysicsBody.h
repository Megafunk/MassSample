// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MSMassPhysicsTypes.h"
#include "MSPhysicsBody.generated.h"

/**
 * Ultra experimental, I probably do things wrong with how I init the chaos bodies.
 * Expect this to change a lot
 */
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMSPhysicsBody : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;


	// Whether mass sends transform data to chaos or vice versa
	UPROPERTY(EditAnywhere)
	bool bChaosToMass = false;

	// Physics sim. Probably not wise to have the sim operate on things we want to set directly from Mass.
	UPROPERTY(EditAnywhere,meta=(EditCondition="bChaosToMass"))
	bool bSimulatesPhysics = false;


	
	UPROPERTY(EditAnywhere)
	bool bHasGravity = false;

	// Used to force create a physics body. Hardcoded to a single capsule
	UPROPERTY(EditAnywhere,AdvancedDisplay)
	bool bManualCollisionSettingsAndGeo = false;
	UPROPERTY(EditAnywhere,AdvancedDisplay,meta=(EditCondition="bManualCollisionSettingsAndGeo"))
	FSharedCollisionSettingsFragment CollisionSettings;
};
