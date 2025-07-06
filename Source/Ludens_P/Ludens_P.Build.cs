// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ludens_P : ModuleRules
{
	public Ludens_P(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
