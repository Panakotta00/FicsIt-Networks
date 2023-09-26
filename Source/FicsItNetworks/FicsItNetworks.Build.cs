// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworks : ModuleRules
{
    public FicsItNetworks(ReadOnlyTargetRules Target) : base(Target)
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
            "ReplicationGraph"
		});
	    PrivateDependencyModuleNames.AddRange(new string[] { "SlateNullRenderer" });

        if (Target.Type == TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[] {"OnlineBlueprintSupport", "AnimGraph"});
		}
        PublicDependencyModuleNames.AddRange(new string[] {"FactoryGame", "SML"});

        var thirdPartyFolder = Path.Combine(ModuleDirectory, "../../ThirdParty");
        PublicIncludePaths.Add(Path.Combine(thirdPartyFolder, "include"));
        
        PublicIncludePaths.Add("Public");
        
        var platformName = Target.Platform.ToString();
        var libraryFolder = Path.Combine(thirdPartyFolder, platformName);
        
        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "eris.lib"));
        
        bEnableExceptions = true;
        bUseRTTI = true;
		
        CppStandard = CppStandardVersion.Cpp17;

        var factoryGamePchPath = new DirectoryReference(Path.Combine(Target.ProjectFile.Directory.ToString(), "Source", "FactoryGame", "Public", "FactoryGame.h"));
        PrivatePCHHeaderFile = factoryGamePchPath.MakeRelativeTo(new DirectoryReference(ModuleDirectory));
    }
}
