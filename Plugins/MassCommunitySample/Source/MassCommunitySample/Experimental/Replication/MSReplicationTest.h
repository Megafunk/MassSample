// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassProcessor.h"
#include "MassReplicationProcessor.h"
#include "MassReplicationTransformHandlers.h"
#include "MassReplicationTypes.h"
#include "MSReplicationTest.generated.h"

/*
 *Replication is hard! I'm trying to mimic the setup for crowd replication for some of the projectile code.
 *
 *So far I have this as my rough guess of how this is supposed to be set up:
 */



/*
 *AGENT SETUP
 *
 */


USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FReplicatedProjectileAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

	//the actual data we are replicating
	UPROPERTY(Transient)
	FColor CurrentColor;

};

/** Fast array item for efficient agent replication. Remember to make this dirty if any FReplicatedProjectileAgent member variables are modified */
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FProjectileFastArrayItem : public FMassFastArrayItemBase
{
	GENERATED_BODY()

	FProjectileFastArrayItem() = default;
	FProjectileFastArrayItem(const FReplicatedProjectileAgent& InAgent, const FMassReplicatedAgentHandle InHandle)
		: FMassFastArrayItemBase(InHandle)
		, Agent(InAgent)
	{}

	/** This typedef is required to be provided in FMassFastArrayItemBase derived classes (with the associated FReplicatedAgentBase derived class) */
	typedef FReplicatedProjectileAgent FReplicatedAgentType;

	UPROPERTY()
	FReplicatedProjectileAgent Agent;
};




/*
 *BUBBLE SETUP
 *
 */


class MASSCOMMUNITYSAMPLE_API FMassProjectileClientBubbleHandler : public TClientBubbleHandlerBase<FProjectileFastArrayItem>
{
public:
	typedef TClientBubbleHandlerBase<FProjectileFastArrayItem> Super;
	
	typedef TMassClientBubbleTransformHandler<FProjectileFastArrayItem> FMassClientBubbleTransformHandler;

	FMassProjectileClientBubbleHandler()
		: TransformHandler(*this)
	{}
	
	FMassClientBubbleTransformHandler TransformHandler;


//TODO: karl actually make these
protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override {};
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override {};

	void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FReplicatedProjectileAgent& Item) const {};
#endif //UE_REPLICATION_COMPILE_CLIENT_CODE

#if WITH_MASSGAMEPLAY_DEBUG && WITH_EDITOR
	virtual void DebugValidateBubbleOnServer() override {};
	virtual void DebugValidateBubbleOnClient() override {};
#endif // WITH_MASSGAMEPLAY_DEBUG





	
};

/** Mass client bubble, there will be one of these per client and it will handle replicating the fast array of Agents between the server and clients */
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FMassProjectileClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

	FMassProjectileClientBubbleSerializer()
	{
		Bubble.Initialize(Projectiles, *this);
	};
		
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FProjectileFastArrayItem, FMassProjectileClientBubbleSerializer>(Projectiles, DeltaParams, *this);
	}

	
	FMassProjectileClientBubbleHandler Bubble;

protected:
	/** Fast Array of Agents for efficient replication. Maintained as a freelist on the server, to keep index consistency as indexes are used as Handles into the Array 
	 *  Note array order is not guaranteed between server and client so handles will not be consistent between them, FMassNetworkID will be.
	 */
	UPROPERTY(Transient)
	TArray<FProjectileFastArrayItem> Projectiles;
};

template<>
struct TStructOpsTypeTraits<FMassProjectileClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMassProjectileClientBubbleSerializer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithCopy = false,
	};
};



