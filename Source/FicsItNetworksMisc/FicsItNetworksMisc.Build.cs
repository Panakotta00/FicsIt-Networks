using UnrealBuildTool;

public class FicsItNetworksMisc : ModuleRules
{
    public FicsItNetworksMisc(ReadOnlyTargetRules Target) : base(Target)
    {
    	bLegacyPublicIncludePaths = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bWarningsAsErrors = true;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "FactoryGame",
                "FicsItReflection",
                "SML"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Json",
                "UMG",
            }
        );
    }
}