// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


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

		if constexpr (std::is_const_v<T>)
		{
			Access = EMassFragmentAccess::ReadOnly;
		}
		if constexpr (std::is_pointer_v<T>)
		{
			Presence = EMassFragmentPresence::Optional;
		}
		if constexpr (std::is_signed_v<T>)
		{
			Presence = EMassFragmentPresence::None;
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
	FMassEntityQuery Query(const TSharedRef<FMassEntityManager>& EntityManager)
	{
		FMassEntityQuery Query;
		Query.Initialize(EntityManager);
		(MSMassUtils::AddAnyFragment<TFragments>(Query, *TFragments::StaticStruct()), ...);
		return Query;
	}



}
