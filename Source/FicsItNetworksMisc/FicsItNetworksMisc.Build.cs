using UnrealBuildTool;

public class FicsItNetworksMisc : ModuleRules
{
    public FicsItNetworksMisc(ReadOnlyTargetRules Target) : base(Target)
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