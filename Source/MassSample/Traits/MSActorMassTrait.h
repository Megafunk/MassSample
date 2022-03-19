// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MSActorMassTrait.generated.h"

/**
 * 
 */

UCLASS(meta = (DisplayName = "Sample Actor Trait"))
class MASSSAMPLE_API UMSActorMassTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
	
	UPROPERTY(EditAnywhere)
	AActor* Actor;
	UPROPERTY(EditAnywhere)
	bool bIsOwnedByMass;
	
};

UCLASS()
class MASSSAMPLE_API UMSActorMassInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()
protected:

	UMSActorMassInitializer();
	
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};
