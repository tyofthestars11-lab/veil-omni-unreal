// VEIL: THE LAST SIGNAL - editor target rules.
using UnrealBuildTool;
using System.Collections.Generic;

public class VEILEditorTarget : TargetRules
{
	public VEILEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// NO YELLOW, NO RED applies here too, but UBT refuses bWarningsAsErrors on
		// an editor target that shares build products with the installed engine
		// ("VEILEditor modifies the values of properties ... not allowed"). The gate
		// is therefore enforced on the GAME target, which owns its products, and the
		// editor build is held to zero warnings by the same build script check.
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("VEIL");
	}
}
