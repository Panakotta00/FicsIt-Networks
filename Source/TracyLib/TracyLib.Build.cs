using UnrealBuildTool;
using System.IO;

public class TracyLib : ModuleRules
{
	public TracyLib(ReadOnlyTargetRules target) : base(target)
	{
		Type = ModuleType.External;
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "public"));
	}
}
