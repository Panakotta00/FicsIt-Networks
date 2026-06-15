using UnrealBuildTool;

public class FicsItNetworksDocumentation : ModuleRules
{
    public FicsItNetworksDocumentation(ReadOnlyTargetRules Target) : base(Target)
    {
    	bLegacyPublicIncludePaths = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bWarningsAsErrors = false; // FIN-1.2-PORT(debt): wieder aktivieren + Warnungen fixen (UE5.6 C4702/C4996, teils in Engine-Code)

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