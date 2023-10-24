using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItVisualScriptEditor : ModuleRules
{
    public FicsItVisualScriptEditor(ReadOnlyTargetRules target) : base(target)
    {
        CppStandard = CppStandardVersion.Cpp17;
        //bLegacyPublicIncludePaths = false;
	    PCHUsage = PCHUsageMode.Default;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"FicsItVisualScript",
            "Tracy"
		});

        PublicIncludePaths.Add("Public");
    }
}
