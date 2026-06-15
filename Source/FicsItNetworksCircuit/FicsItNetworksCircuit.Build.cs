using UnrealBuildTool;

public class FicsItNetworksCircuit : ModuleRules
{
    public FicsItNetworksCircuit(ReadOnlyTargetRules Target) : base(Target)
    {
    	bLegacyPublicIncludePaths = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;
        bWarningsAsErrors = false; // FIN-1.2-PORT(debt): wieder aktivieren + Warnungen fixen (UE5.6 C4702/C4996, teils in Engine-Code)

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "FactoryGame",
                "SML",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "FicsItReflection",
            }
        );
    }
}