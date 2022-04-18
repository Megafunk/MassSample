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
				"InputCore",
				
				
				
				
				
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
				"MassGameplayDebug",
				"MassSignals",
				"MassCrowd",
				"MassActors",
				"MassRepresentation",
				"MassReplication",
				"MassNavigation",
				//needed for replication setup
				"NetCore",
				"AIModule",
				"MassAIBehavior",

				"ZoneGraph",
				"ZoneGraphDebug",
				"MassZoneGraphNavigation",
				"ZoneGraphAnnotations",
				"MassSimulation",
				
				"MassGameplayDebug",
				"Niagara",
				"DeveloperSettings",
				"GeometryCore",
				"AITestSuite",
				
				"StateTreeModule",
				
				//todo: maybe do thee editor only stuff on another module?
				"UnrealEd"
			}
		);
	}
}
