using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworks : ModuleRules
{
    public FicsItNetworks(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

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
            "Eris",
            "Tracy"
		});
	    PrivateDependencyModuleNames.AddRange(new string[] { "SlateNullRenderer" });

        if (target.Type == TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[] {"OnlineBlueprintSupport", "AnimGraph"});
		}

        PublicIncludePaths.Add("Public");

        bEnableExceptions = true;
        bUseRTTI = true;
		
        CppStandard = CppStandardVersion.Cpp17;

        if (target.ProjectFile != null)
        {
	        var factoryGamePchPath = new DirectoryReference(Path.Combine(target.ProjectFile.Directory.ToString(),
		        "Source", "FactoryGame", "Public", "FactoryGame.h"));
	        PrivatePCHHeaderFile = factoryGamePchPath.MakeRelativeTo(new DirectoryReference(ModuleDirectory));
        }
    }
}
