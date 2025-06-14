using UnrealBuildTool;

public class FicsItReflection : ModuleRules
{
    public FicsItReflection(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bWarningsAsErrors = true;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "FactoryGame",
                "CoreUObject",
                "Engine",
                "SML",
                "FicsItLogLibrary",
                "TracyLib",
                "SlateCore",
            }
        );
    }
}