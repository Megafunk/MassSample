#include "MSProjectileSubsystem.h"

void UMSProjectileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
		
	MassSubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		
	// ForEachObjectOfClass(UMassProcessor::StaticClass(), [&](UObject* Obj)
	// {
	// 	UMassProcessor* mycdo = Cast<UMassProcessor>(Obj->GetClass()->GetDefaultObject());
	// 	
	// 	if(!mycdo->ShouldAutoAddToGlobalList())
	// 	{
	// 		mycdo->SetShouldAutoRegisterWithGlobalList(true);
	// 	}
	// }, true);
	
		
}

void UMSProjectileSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{

		
}

void UMSProjectileSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

