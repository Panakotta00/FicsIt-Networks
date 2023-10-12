// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class Tracy : ModuleRules
{
    public Tracy(ReadOnlyTargetRules target) : base(target)
    {
		CppStandard = CppStandardVersion.Cpp17;
		
        bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "TracyLib"
		});
		
		PublicIncludePaths.Add("Public");
		
		PublicDefinitions.Add("TRACY_ENABLE=1");
		PublicDefinitions.Add("TRACY_EXPORTS=1");
		PublicDefinitions.Add("TRACY_ON_DEMAND=1");
		//PublicDefinitions.Add("TRACY_CALLSTACK=1");
		//PublicDefinitions.Add("TRACY_HAS_CALLSTACK=1");
    }
}
