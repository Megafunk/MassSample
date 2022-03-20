#pragma once
#include <ThirdParty/Vulkan/Include/vulkan/vulkan_core.h>

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationProcessor.h"
#include "EntitySystem/MovieSceneEntityIDs.h"

#include "Field/FieldSystemNodes.h"
#include "MSProjectileSubsystem.generated.h"




UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Mass Sample Projectiles"))
class UMassProjectileSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**Default niagara bullet manager */
	UPROPERTY(config, EditAnywhere, Category = "Niagara")
	TSubclassOf<class UNiagaraSystem> DefaultNiagaraBulletManager;
	
};




//TODO: Might get around using this to store niagara fragments in the future
//It's still useful to serve as a place anything can get projectile info though...
UCLASS()
class UMSProjectileSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
protected:
	
	UMassEntitySubsystem* MassSubsystem;
	

	
	FMassArchetypeHandle BulletArchetype;


	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	virtual void Tick(float DeltaTime) override;
public:

	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBattlementECSSubsystem, STATGROUP_Tickables); }

	//UFUNCTION(BlueprintCallable)
	//int32 SpawnProjectileEntity(FTransform Position, FVector Velocity, bool bRicochet);
	
};
