// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassSample : ModuleRules
{
	public MassSample(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PublicDefinitions.Add("WITH_MASSGAMEPLAY_DEBUG=1");
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MassEntity",
			"StructUtils",
			"MassCommon",
			"MassMovement",
			"MassActors",
			"MassSpawner",
			"MassRepresentation",
			"MassSignals",
			"MassCrowd",
			"MassReplication",
			"ZoneGraph",
			"MassZoneGraphNavigation"

			
		});
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
