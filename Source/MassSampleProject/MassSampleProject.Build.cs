// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassSampleProject : ModuleRules
{
	public MassSampleProject(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicIncludePaths.AddRange(
            new string[] {
                "MassSampleProject"
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
		
	}
}
