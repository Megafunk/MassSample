// Fill out your copyright notice in the Description page of Project Settings.


#include "MassExecutorAPI.h"

#include "MassEntitySubsystem.h"

void UMassExecutorAPI::Deinitialize() {
	Super::Deinitialize();
}

void UMassExecutorAPI::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
	
	auto EntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();

	ĘntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();

}
