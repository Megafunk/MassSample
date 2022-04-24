// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Misc/MSBPFunctionLibarary.h"
#include "UObject/Interface.h"
#include "MassProjectileHitInterface.generated.h"

UINTERFACE()
class UMassProjectileHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MASSSAMPLE_API IMassProjectileHitInterface
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent, Category="Mass")
	void ProjectileHit(FEntityHandleWrapper Entity, FHitResult HitResult);

	

};
