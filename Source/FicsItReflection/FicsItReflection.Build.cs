using UnrealBuildTool;

public class FicsItReflection : ModuleRules
{
    public FicsItReflection(ReadOnlyTargetRules Target) : base(Target)
    {
    	bLegacyPublicIncludePaths = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bWarningsAsErrors = false; // FIN-1.2-PORT(debt): wieder aktivieren + Warnungen fixen (UE5.6 C4702/C4996, teils in Engine-Code)
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