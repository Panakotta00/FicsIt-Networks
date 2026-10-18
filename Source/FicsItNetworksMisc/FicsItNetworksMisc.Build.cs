using UnrealBuildTool;

public class FicsItNetworksMisc : ModuleRules
{
    public FicsItNetworksMisc(ReadOnlyTargetRules Target) : base(Target)
    {
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
                "GameplayTags",
                "SML"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Json",
                "ReliableMessaging",
                "UMG",
            }
        );
    }
}