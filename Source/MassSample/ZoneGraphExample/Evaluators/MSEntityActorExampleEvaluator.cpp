// Fill out your copyright notice in the Description page of Project Settings.


#include "ZoneGraphExample/Evaluators/MSEntityActorExampleEvaluator.h"

#include "MassActorSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "ZoneGraphExample/MSEntityActorExampleComponent.h"
#include "ZoneGraphExample/MSEntityActorExampleSubsystem.h"


bool FMSEntityActorExampleEvaluator::Link(FStateTreeLinker& Linker)
{

	
	Linker.LinkExternalData(MSEntityActorExampleSubsystemHandle);
	

	/* This is an example of an output from an AI StateTree Evaluator, the string retrieved is obtained from the component attached to the actor that is in turn obtained by,
	 * setting the entityHandle to the actor in "UMSEntityActorExampleSubsystem".
	 */
	Linker.LinkInstanceDataProperty(TestStringHandle, STATETREE_INSTANCEDATA_PROPERTY(FMSEntityActorExampleEvaluatorInstanceData, TestString));

	return true;
}

void FMSEntityActorExampleEvaluator::Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	UMSEntityActorExampleSubsystem& MSEntityActorExampleSubsystem = Context.GetExternalData(MSEntityActorExampleSubsystemHandle);


	FString TestString = Context.GetInstanceData(TestStringHandle);

	UActorComponent* NewComponent = MSEntityActorExampleSubsystem.EntityToComponentMap.FindRef(static_cast<FMassStateTreeExecutionContext&>(Context).GetEntity());

	if (NewComponent)
	{
		if (UActorComponent* OutComponent = Cast<UMSEntityActorExampleComponent>(NewComponent))
		{
			

			if (UMSEntityActorExampleComponent* EntityActorExampleComponent = Cast<UMSEntityActorExampleComponent>(OutComponent))
			{
				TestString = EntityActorExampleComponent->TestString;
			}
	
			UE_LOG(LogTemp, Warning, TEXT("Found 'UMSEntityActorExampleComponent' Component: TestString: %s"), *TestString);
		}
	}

	AActor* NewActor = MSEntityActorExampleSubsystem.ActorSubsystem->GetActorFromHandle(static_cast<FMassStateTreeExecutionContext&>(Context).GetEntity());

	if (NewActor)
	{

		if (NewActor->GetComponentByClass(UMSEntityActorExampleComponent::StaticClass()))
		{
			UActorComponent* OutComponent = NewActor->GetComponentByClass(UMSEntityActorExampleComponent::StaticClass());

			UE_LOG(LogTemp, Warning, TEXT("Found Owning Actor of 'UMSEntityActorExampleComponent'"));

			if (UMSEntityActorExampleComponent* EntityActorExampleComponent = Cast<UMSEntityActorExampleComponent>(OutComponent))
			{
				TestString = EntityActorExampleComponent->TestString;
			}
		}
	}
}
