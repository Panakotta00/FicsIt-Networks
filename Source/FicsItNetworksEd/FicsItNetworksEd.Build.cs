// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItNetworksEd : ModuleRules
{
    public FicsItNetworksEd(ReadOnlyTargetRules Target) : base(Target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	    CppStandard = CppStandardVersion.Cpp20;
        bLegacyPublicIncludePaths = false;
        bWarningsAsErrors = true;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "FicsItNetworks",
            "UnrealEd",
            "Localization",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"FicsItReflection",
		});
		
		var factoryGamePchPath = new DirectoryReference(Path.Combine(Target.ProjectFile.Directory.ToString(), "Source", "FactoryGame", "Public", "FactoryGame.h"));
		PrivatePCHHeaderFile = factoryGamePchPath.MakeRelativeTo(new DirectoryReference(ModuleDirectory));
    }
}
