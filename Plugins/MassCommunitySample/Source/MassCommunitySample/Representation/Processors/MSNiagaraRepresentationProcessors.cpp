// Fill out your copyright notice in the Description page of Project Settings.

#include "MSNiagaraRepresentationProcessors.h"
#include "MassCommonFragments.h"
#include "MassSignalSubsystem.h"
#include "Common/Fragments/MSFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Representation/Fragments/MSRepresentationFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MSNiagaraRepresentationProcessors)

UMSNiagaraRepresentationProcessors::UMSNiagaraRepresentationProcessors()
{
	//We don't care about rendering on the dedicated server!
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	//join the other representation processors in their existing group
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	
	// I made a personal engine change where I added "PostUpdate" to the mass processing phases.
	// this is to safely execute mass niagara systems AFTER animations and then BEFORE they get pushed to the render thread by their niagara component
	// In the future I should  just do something that uses the process dependencies to ensure their order rather than ticking groups
	// ProcessingPhase = EMassProcessingPhase::PostUpdate;
}

void UMSNiagaraRepresentationProcessors::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	PositionToNiagaraFragmentQuery.Initialize(EntityManager);
	PositionToNiagaraFragmentQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	PositionToNiagaraFragmentQuery.AddSharedRequirement<FMSSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
	PositionToNiagaraFragmentQuery.RegisterWithProcessor(*this);
}

// todo-performance separate setup for rarely moving pieces?
void UMSNiagaraRepresentationProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Query mass for transform data
	PositionToNiagaraFragmentQuery.ForEachEntityChunk( Context, [&,this](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PositionToNiagara);
		const int32 QueryLength = Context.GetNumEntities();

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		FMSSharedNiagaraSystemFragment& SharedNiagaraSystemFragment = Context.GetMutableSharedFragment<FMSSharedNiagaraSystemFragment>();

		AMSNiagaraActor* ManagerActor = SharedNiagaraSystemFragment.NiagaraManagerActor.Get();
		
		if (!ManagerActor) {
			return;
		}

		int32 ArrayResizeAmount = ManagerActor->IteratorOffset + QueryLength;

		ManagerActor->IteratorOffset += QueryLength;
		ManagerActor->ParticlePositions.SetNumUninitialized(ArrayResizeAmount);
		ManagerActor->ParticleOrientations.SetNumUninitialized(ArrayResizeAmount);

		FVector* PositionsDataArray = ManagerActor->ParticlePositions.GetData();
		FQuat4f* DirectionsDataArray = ManagerActor->ParticleOrientations.GetData();


		for (int32 i = 0; i < QueryLength; ++i)
		{
			// this is needed because there are multiple chunks for each shared niagara system
			const int32 ArrayPosition = i + ManagerActor->IteratorOffset - QueryLength;
			auto& Transform = Transforms[i].GetTransform();

			PositionsDataArray[ArrayPosition] = Transform.GetTranslation();
			DirectionsDataArray[ArrayPosition] = (FQuat4f)Transform.GetRotation();
		}
	});

	// with our nice new data, we push to the actual niagara components in the world!
	EntityManager.ForEachSharedFragment<FMSSharedNiagaraSystemFragment>([](FMSSharedNiagaraSystemFragment& SharedNiagaraFragment)
	{
		AMSNiagaraActor* NiagaraActor = SharedNiagaraFragment.NiagaraManagerActor.Get();
		
		if (!NiagaraActor) {
			return;
		}

		// UE_LOG( LogTemp, Error, TEXT("Niagara array length for %s is %i"),*NiagaraActor->GetName(),SharedNiagaraFragment.NiagaraManagerActor->ParticlePositions.Num());
		if (UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent())
		{
			// If the offset is 0, we didn't iterate anything and should reset our arrays (could do an evil setnumunsafe?)
			if (NiagaraActor->IteratorOffset == 0)
			{
				NiagaraActor->ParticlePositions.Reset();
				NiagaraActor->ParticleOrientations.Reset();
			}

			static FName ParticlePositionsName = "MassParticlePositions";

			static FName ParticleOrientationsParameterName = "MassParticleOrientations";

			// congratulations to me (karl) for making SetNiagaraArrayVector public in an engine PR (he's so cool) (wow)
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, ParticlePositionsName,
			                                                                 NiagaraActor->ParticlePositions);
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(NiagaraComponent, ParticleOrientationsParameterName,
			                                                               NiagaraActor->ParticleOrientations);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("projectile manager %s was invalid during array push!"), *NiagaraActor->GetName());
		}

		// Let's prepare the manager actor to accept new data next frame,
		// this also ensures that if no entities are left we will reset the array data
		
		NiagaraActor->IteratorOffset = 0;
	});
}


UMSNiagaraRepresentationSpawnProcs::UMSNiagaraRepresentationSpawnProcs()
{
	//We don't care about rendering on the dedicated server!
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	//join the other representation processors in their existing group
	ExecutionOrder.ExecuteAfter.Add(TEXT("MSNiagaraRepresentationProcessors"));
}

void UMSNiagaraRepresentationSpawnProcs::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FSharedNiagaraSystemSpawnFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMSNiagaraRepresentationSpawnProcs::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                                                        FMassSignalNameLookup& EntitySignals)
{
	//query mass for transform data
	EntityQuery.ForEachEntityChunk( Context, [&,this](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_SpawnPositionToNiagara);

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const FSharedNiagaraSystemSpawnFragment& SharedNiagaraFragment = Context.GetSharedFragment<FSharedNiagaraSystemSpawnFragment>();

		check(SharedNiagaraFragment.NiagaraManagerActor.Get()->GetNiagaraComponent())


		UNiagaraComponent* NiagaraComponent = SharedNiagaraFragment.NiagaraManagerActor.Get()->GetNiagaraComponent();

		// There is a world where this signal execution is done multiple times 
		TArray<FVector> LocationsArray;
		TArray<FVector> NormalsArray;
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			LocationsArray.Add(Transforms[i].GetTransform().GetLocation());
			NormalsArray.Add(-Transforms[i].GetTransform().GetRotation().GetForwardVector());

			UE_LOG(LogTemp, Error, TEXT("UMSNiagaraRepresentationSpawnProcs: projectile manager niagara system %s iterated on! with chunk length "), *NiagaraComponent->GetName());
		}

		static FName HitPositionsName = "HitPositions";
		static FName HitNormalsName = "HitNormals";
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, HitPositionsName, LocationsArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, HitNormalsName, NormalsArray);
		//NiagaraComponent->SetNiagaraVariableInt("System.NumToSpawnThisFrame",Context.GetNumEntities());
	});
}

void UMSNiagaraRepresentationSpawnProcs::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& Manager)
{
	Super::InitializeInternal(Owner, Manager);
	
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	if (SignalSubsystem)
	{
		SubscribeToSignal(*SignalSubsystem, MassSample::Signals::OnEntityHitSomething);
	}
}
