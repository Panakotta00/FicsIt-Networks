using UnrealBuildTool;
using System.IO;

public class Eris : ModuleRules
{
	public Eris(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		bEnableExceptions = true;
		bUseRTTI = true;
		bUseUnity = false;
		PCHUsage = PCHUsageMode.NoPCHs;
		// bNoCommonPCHForModules = true;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core"
		});

		bEnableUndefinedIdentifierWarnings = false;
		
		var thirdPartyFolder = Path.Combine(PluginDirectory, "ThirdParty");
		PublicIncludePaths.Add(Path.Combine(thirdPartyFolder, "eris/src"));

		PublicIncludePaths.Add("Public");

		PublicDefinitions.AddRange(new string[] { "LUA_LIB", "LUA_API=ERIS_API" });

		if (target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.AddRange(new string[] { "LUA_BUILD_AS_DLL" });
		}
	}
}
