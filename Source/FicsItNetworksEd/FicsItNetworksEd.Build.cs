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
        bLegacyPublicIncludePaths = true;
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
		
		// FIN-1.2-PORT: Custom PCH auf FactoryGame.h entfernt — bootstrapt Core in 1.2/UE5.6
		// nicht mehr sauber (FString/FName undefiniert in LowLevelMemTracker.h). Beide Ed-.cpp
		// includen ihre Header selbst (IWYU), daher Shared-PCH (PCHUsage oben) ausreichend.
    }
}
