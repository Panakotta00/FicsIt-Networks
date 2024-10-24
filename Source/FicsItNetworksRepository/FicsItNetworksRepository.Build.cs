using UnrealBuildTool;

public class FicsItNetworksRepository : ModuleRules
{
    public FicsItNetworksRepository(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HTTP",
                "Json",
                "UMG",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "SML",
                "InputCore",
                "ApplicationCore",
                "Slate",
                "SlateCore",
            }
        );

        if (Target.Type == TargetType.Game)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "WebBrowser",
            });
        }
    }
}