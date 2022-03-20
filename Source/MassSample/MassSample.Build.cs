// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassSample : ModuleRules
{
	public MassSample(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicIncludePaths.AddRange(
            new string[] {
                "MassSample"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
            }
        );

        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore" 
			}
		);

        PrivateDependencyModuleNames.AddRange(
			new string[] {
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
				"MassGameplayDebug",
				"MassZoneGraphNavigation", 
				"Niagara",
				"DeveloperSettings"
			}
		);
	}
}
