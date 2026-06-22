using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworks : ModuleRules
{
    public FicsItNetworks(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = true;

        bEnableExceptions = true;
        bWarningsAsErrors = true;

        CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[] {
            "VorbisAudioDecoder", // FIN-1.2-PORT: FVorbisAudioInfo lebt jetzt im eigenen Modul
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
