// VEIL: THE LAST SIGNAL - module build rules. Tyree / tyofthestarz.
using UnrealBuildTool;

public class VEIL : ModuleRules
{
	public VEIL(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"UMG",
			"Slate",
			"SlateCore"
		});

		// UnrealEd is editor-only and is used for exactly one thing: baking the district
		// into the LEVEL as saved actors. Everything else in this module is runtime.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
	}
}
