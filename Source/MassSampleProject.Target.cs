// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MassSampleProjectTarget : TargetRules
{
	public MassSampleProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange(new string[] { "MassSampleProject" });

		DefaultBuildSettings = BuildSettingsVersion.V5;
	}
}
