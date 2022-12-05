// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MassSampleProjectTarget : TargetRules
{
	public MassSampleProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "MassSampleProject" });

		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

	}
}
