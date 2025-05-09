using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItVisualScript : ModuleRules
{
    public FicsItVisualScript(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	    CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;
		bWarningsAsErrors = true;

	    OptimizeCode = CodeOptimization.Never;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"JsonUtilities",
			"UMG",
			"SlateCore",
			"Slate",
			"ApplicationCore",
			"InputCore",
			"FactoryGame",
			"FicsItLogLibrary",
            "FicsItNetworks",
            "FicsItNetworksComputer",
            "FicsItNetworksCircuit",
            "FicsItReflection",
            "Tracy"
		});

        PublicIncludePaths.Add("Public");
    }
}
