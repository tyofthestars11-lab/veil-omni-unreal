// VEIL: THE LAST SIGNAL - game target rules.
using UnrealBuildTool;
using System.Collections.Generic;

public class VEILTarget : TargetRules
{
	public VEILTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		// NO YELLOW, NO RED is enforced by Tools\veil_build_gate.ps1, not by this
		// property: UBT refuses bWarningsAsErrors on any target whose build products
		// are shared with the installed engine ("VEIL has build products in common
		// with UnrealGame"). The gate script parses the compiler output instead and
		// fails the build on a single warning, which is the same rule held one level
		// up - and it cannot silently stop applying.
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("VEIL");
	}
}
