using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworksLua : ModuleRules
{
    public FicsItNetworksLua(ReadOnlyTargetRules target) : base(target)
    {
        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;
        bUseRTTI = true;
        //bLegacyPublicIncludePaths = false;
	    PCHUsage = PCHUsageMode.Default;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"FactoryGame",
			"SML",
            "FicsItNetworks",
            "Eris",
            "Tracy",
            "FicsItLogLibrary",
            "FicsItReflection",
		});

        PublicIncludePaths.Add("Public");
    }
}
