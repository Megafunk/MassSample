// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraRepresentationProcessors.h"

#include "NiagaraComponent.h"
#include "MSNiagaraActor.h"

UMSNiagaraRepresentationProcessors::UMSNiagaraRepresentationProcessors()
{
	bAutoRegisterWithProcessingPhases = true;
	//We don't care about rendering on the dedicated server!
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	//join the other representation processors in their existing group
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
}

void UMSNiagaraRepresentationProcessors::ConfigureQueries()
{
	PositionToNiagaraFragmentQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	PositionToNiagaraFragmentQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
}


void UMSNiagaraRepresentationProcessors::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
		//Let's prepare the shared fragments to accept new data!
		EntitySubsystem.ForEachSharedFragment<FSharedNiagaraSystemFragment>([&,this](FSharedNiagaraSystemFragment& SharedNiagaraFragment)
		{
			SharedNiagaraFragment.IteratorOffset = 0;
		});
	
		//query mass for transform data
		PositionToNiagaraFragmentQuery.ForEachEntityChunk(EntitySubsystem,Context,
			[&,this](FMassExecutionContext& Context)
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PositionToNiagara);
			const int32 QueryLength = Context.GetNumEntities();
				
			const auto& Transforms = Context.GetFragmentView<FTransformFragment>().GetData();
			auto& SharedNiagaraFragment = Context.GetMutableSharedFragment<FSharedNiagaraSystemFragment>();
				
			AMSNiagaraActor* NiagaraActor =  SharedNiagaraFragment.NiagaraManagerActor.Get();
				
			if(!NiagaraActor) return;
				
			//todo-performance: shrink this with GC timing?
			bool bAllowShrinking = false;

			//todo-performance this should also probably not even happen if the total array num isn't going to change 
			int32 ArrayResizeAmount = SharedNiagaraFragment.IteratorOffset + QueryLength;
				
			//todo-performance if we want multithreading does this need to happen on another foreach?
			//I did have this parrallelfor'd before but 
			SharedNiagaraFragment.NiagaraManagerActor.Get()->ParticlePositions.SetNumUninitialized(ArrayResizeAmount,bAllowShrinking);
			SharedNiagaraFragment.NiagaraManagerActor.Get()->ParticleDirectionVectors.SetNumUninitialized(ArrayResizeAmount,bAllowShrinking);
				
			SharedNiagaraFragment.IteratorOffset += QueryLength;
				
			for (int32 i = 0; i < QueryLength; ++i)
			{
				 //this is needed because there are multiple chunks for each shared niagara system 
				 const int32 ArrayPosition = i + SharedNiagaraFragment.IteratorOffset - QueryLength;
				 NiagaraActor->ParticlePositions[ArrayPosition] = Transforms[i].GetTransform().GetTranslation();
				 NiagaraActor->ParticleDirectionVectors[ArrayPosition] = Transforms[i].GetTransform().GetRotation().GetForwardVector();
				//temp log to double check iteration order etc
				//UE_LOG( LogTemp, Error, TEXT("projectile manager niagara system %s iterated on! with chunk length %i "),*NiagaraActor->GetName(),QueryLength);
			}
		});

	//with our nice new data, we push to the actual niagara components in the world!
	EntitySubsystem.ForEachSharedFragment<FSharedNiagaraSystemFragment>([](FSharedNiagaraSystemFragment& SharedNiagaraFragment)
	{
		
		AMSNiagaraActor* NiagaraActor =  SharedNiagaraFragment.NiagaraManagerActor.Get();
		
		//UE_LOG( LogTemp, Error, TEXT("Niagara array length for %s is %i"),*NiagaraActor->GetName(),SharedNiagaraFragment.NiagaraManagerActor->ParticlePositions.Num());
		
		if(UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent())
		{
			
			//congratulations to me (karl) for making SetNiagaraArrayVector public in an engine PR (he's so cool) (wow)
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent,"MassParticlePositions",NiagaraActor->ParticlePositions);
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent,"MassParticleDirectionVectors",NiagaraActor->ParticleDirectionVectors);
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT("projectile manager %s was invalid during array push!"),*NiagaraActor->GetName());
		}
		
	});
	
}