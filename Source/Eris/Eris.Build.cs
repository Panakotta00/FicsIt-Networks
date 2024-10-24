using UnrealBuildTool;
using System.IO;

public class Eris : ModuleRules
{
	public Eris(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Cpp17;
		bEnableExceptions = true;
		bUseRTTI = true;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core"
		});
		
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
