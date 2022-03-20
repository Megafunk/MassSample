// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraRepresentationProcessors.h"

#include "NiagaraSystem.h"

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
	PositionToNiagaraQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	PositionToNiagaraQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
}


void UMSNiagaraRepresentationProcessors::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{


	
		// //todo: pray that this behaves the same with shared fragments for now
		// auto TotalMatchingEntities = PositionToNiagaraQuery.GetNumMatchingEntities(EntitySubsystem);
		// //todo-performance: shrink this with GC timing?
		// bool bAllowShrinking = false;
		// NiagaraProjectileManagerActor->ProjectilePositions.SetNumUninitialized(TotalMatchingEntities,bAllowShrinking);
		// NiagaraProjectileManagerActor->ProjectilePreviousPositions.SetNumUninitialized(TotalMatchingEntities,bAllowShrinking);
		//
		// //fyi: do this after setting array size
		// auto ParticlePositionData = NiagaraProjectileManagerActor->ProjectilePositions.GetData();
		// auto ParticleRotationData = NiagaraProjectileManagerActor->ProjectilePreviousPositions.GetData();

		//this exists because we are stuffing multiple forlooped chunks of entities into one array! that means we must store an offset
		int32 iteratoroffset = 0;

		/* @todo-performance Ordering beneficial? Skip updating stationary bullets part of the array? 
		*/
		
		PositionToNiagaraQuery.ForEachEntityChunk(EntitySubsystem,Context,
			[&,this](FMassExecutionContext& Context)
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PositionToNiagara);
			const int32 QueryLength = Context.GetNumEntities();


				
				//offset by the number of entities we have in this iteration of the foreach
			iteratoroffset += QueryLength;
			
			auto Transforms = Context.GetFragmentView<FTransformFragment>().GetData();
			const UNiagaraSystem* NiagaraSystem = Context.GetSharedFragment<FSharedNiagaraSystemFragment>()
			                                             .NiagaraSystem.Get();

			if(!NiagaraSystem) return;
			UE_LOG( LogTemp, Error, TEXT("projectile manager niagara system %s iterated on!"),*NiagaraSystem->GetName());

			for (int32 i = 0; i < QueryLength; ++i)
			{
				// ParticlePositionData[i + iteratoroffset - QueryLength] = Transforms[i].GetTransform().GetTranslation();
				// ParticleRotationData[i + iteratoroffset - QueryLength] = Transforms[i].GetTransform().GetRotation().GetForwardVector();

				
			}
			
		});
		
		// if(NiagaraProjectileManagerActor->NiagaraProjectileManagerSystem)
		// {
		// 	//congratulations to me (karl) for making this public (he's so cool) (wow)
		// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraProjectileManagerActor->NiagaraProjectileManagerSystem,"ProjectilePositions",NiagaraProjectileManagerActor->ProjectilePositions);
		// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraProjectileManagerActor->NiagaraProjectileManagerSystem,"PrevProjectilePositions",NiagaraProjectileManagerActor->ProjectilePreviousPositions);
		// }
		// else
		// {
		// 	UE_LOG( LogTemp, Error, TEXT("projectile manager niagara system null!"));
		// }
	
}