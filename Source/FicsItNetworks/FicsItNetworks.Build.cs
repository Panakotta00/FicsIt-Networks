using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworks : ModuleRules
{
    public FicsItNetworks(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableExceptions = true;
        bWarningsAsErrors = true;

        CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "InputCore",
            "ReplicationGraph",
            "UMG",
            "AssetRegistry",
            "AnimGraphRuntime",
            "Slate", "SlateCore",
            "AudioPlatformConfiguration",
            "EnhancedInput",
            "GameplayTags",
            "ApplicationCore",
            "Json",
            "Vorbis",
            "Http",
            "OnlineSubsystemUtils",
            "ReplicationGraph",
            "FactoryGame",
            "SML",
            "Tracy",
            "FicsItReflection",
            "FicsItLogLibrary",
            "FicsItNetworksCircuit",
            "FicsItNetworksMisc",
            "FicsItNetworksComputer",
            "FicsItNetworksLua",
		});
    }
}
