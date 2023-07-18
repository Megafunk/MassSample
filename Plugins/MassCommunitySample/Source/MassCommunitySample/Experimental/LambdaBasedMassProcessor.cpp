// Fill out your copyright notice in the Description page of Project Settings.


#include "LambdaBasedMassProcessor.h"


void ULambdaMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	Query.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& InnerContext)
	{
		ExecuteFunction(InnerContext);
	});
}
