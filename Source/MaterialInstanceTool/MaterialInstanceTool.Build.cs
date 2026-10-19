// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MaterialInstanceTool : ModuleRules
{
	public MaterialInstanceTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
			"UnrealEd",
			"AssetTools",
			"ContentBrowser",
			"PropertyEditor",
			"ToolMenus",
			"Projects",
			"EditorScriptingUtilities",
			}
		);
	}
}
