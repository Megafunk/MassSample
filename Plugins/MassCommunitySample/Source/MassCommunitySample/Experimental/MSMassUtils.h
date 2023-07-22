// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LambdaBasedMassProcessor.h"
#include "MassEntityQuery.h"

/**
 * 
 */
namespace MSMassUtils
{

	// Meant for use with variadic templates in the above Query<> 
	template <typename T>
	void AddAnyFragment(FMassEntityQuery& Query, UScriptStruct& Type)
	{
		EMassFragmentAccess Access = EMassFragmentAccess::ReadWrite;
		EMassFragmentPresence Presence = EMassFragmentPresence::All;

		if constexpr (TIsConst<T>::Value)
		{
			Access = EMassFragmentAccess::ReadOnly;
		}
		if constexpr (TIsPointer<T>::Value)
		{
			Presence = EMassFragmentPresence::Optional;
		}

		if constexpr (TIsDerivedFrom<T, FMassFragment>::IsDerived)
		{
			Query.AddRequirement(&Type, Access, Presence);
		}
		else if constexpr (TIsDerivedFrom<T, FMassTag>::IsDerived)
		{
			Query.AddTagRequirement(Type, Presence);
		}
		else if constexpr (TIsDerivedFrom<T, FMassSharedFragment>::IsDerived)
		{
			Query.AddSharedRequirement<T>(Access, Presence);
		}
	}
	
	/**
	 *Template based Mass query
	 * Supports: (for now)
	 * Fragments,Tags, (non const) FMassSharedFragment
	 * Access: readonly/readwrite
	 * If you need the other kinds, just add them to the query later I suppose.
	 */
	template <typename... TFragments>
	FMassEntityQuery Query()
	{
		FMassEntityQuery Query;

		(MSMassUtils::AddAnyFragment<TFragments>(Query, *TFragments::StaticStruct()), ...);
		return Query;
	}



}
