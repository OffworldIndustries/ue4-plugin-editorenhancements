// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OWI_EditorEnhancements : ModuleRules
{
	public OWI_EditorEnhancements(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new []
			{
				"Core"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"DataValidation",
				"UnrealEd",
				"InputCore",
				"ContentBrowser",
				"EditorScriptingUtilities"
			}
		);
	}
}
