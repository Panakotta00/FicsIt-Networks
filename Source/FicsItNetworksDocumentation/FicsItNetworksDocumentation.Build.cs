using UnrealBuildTool;

public class FicsItNetworksDocumentation : ModuleRules
{
    public FicsItNetworksDocumentation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bWarningsAsErrors = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Json",
                "JsonUtilities",
                "FicsItNetworks",
                "FicsItNetworksLua",
                "FicsItReflection",
            }
        );
    }
}