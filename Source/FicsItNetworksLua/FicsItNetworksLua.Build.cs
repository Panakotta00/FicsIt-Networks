using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworksLua : ModuleRules
{
    public FicsItNetworksLua(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;
        bWarningsAsErrors = true;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"FactoryGame",
			"SML",
            "Eris",
            "Tracy",
            "FicsItFileSystem",
            "FicsItLogLibrary",
            "FicsItReflection",
            "FicsItNetworksCircuit",
			"FicsItNetworksComputer",
			"FicsItNetworksMisc",
		});

        PublicIncludePaths.Add("Public");
    }
}
