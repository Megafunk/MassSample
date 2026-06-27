// Some copyright should be here...

using UnrealBuildTool;

public class MassCommunitySample : ModuleRules
{
	public MassCommunitySample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory
			}
		);
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore" ,
				"Chaos"
				
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"MassCore", // This module is new in 5.8
				"MassEntity",
				//"StructUtils", StructUtils is now in the base engine coreuobject module! yay
				"MassCommon",
				"MassMovement",
				"MassActors",
				"MassSpawner",
				"MassGameplayDebug",
				"MassSignals",
				"MassCrowd",
				"MassActors",
				"MassSpawner",
				"MassRepresentation",
				"MassReplication",
				"MassNavigation",
				"MassSimulation",
				//needed for replication setup
				"NetCore",
				"AIModule",

				"ZoneGraph",
				"MassGameplayDebug",
				"MassZoneGraphNavigation", 
				"Niagara",
				"DeveloperSettings",
				"GeometryCore",
				"MassAIBehavior",
				"StateTreeModule",
				"MassLOD",
				"NavigationSystem",
				"Chaos",
				"PhysicsCore",
				"ChaosCore",
				"ChaosSolverEngine", "CADKernel",
				"RHI"
			}
		);
		
		//todo: maybe do thee editor only stuff on another module?

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("CodeView");
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
		
		
		
	}
}
