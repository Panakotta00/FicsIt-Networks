using UnrealBuildTool;
using System.IO;

public class Eris : ModuleRules
{
	public Eris(ReadOnlyTargetRules target) : base(target)
	{
		bLegacyPublicIncludePaths = false;
		CppStandard = CppStandardVersion.Cpp17;
		bEnableExceptions = true;
		bUseRTTI = true;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core"
		});
		
		var thirdPartyFolder = Path.Combine(PluginDirectory, "ThirdParty");
		PrivateIncludePaths.Add(Path.Combine(thirdPartyFolder, "eris/src"));

		PublicIncludePaths.Add("Public");

		PublicDefinitions.AddRange(new string[] { "LUA_BUILD_AS_DLL", "LUA_LIB" });
	}
}
