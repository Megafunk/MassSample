// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSEntityActorExampleComponent.generated.h"



/*
 * This is a test component to show Mass<->Agent actor component setups, refer to "UMSEntityActorExampleSubsystem" for details
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MASSSAMPLE_API UMSEntityActorExampleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMSEntityActorExampleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



	/** This is a test string that will be read by various "MSEntity <-> Actor" things to test and show operation,
	 * for example of helper/functionality involving Entity <-> Actor/Component process "UMSBPFunctionLibarary"**/
	UPROPERTY(EditAnywhere)
	FString TestString = "This is a test string";


	UFUNCTION()
	FString GetTestString();
		
};
