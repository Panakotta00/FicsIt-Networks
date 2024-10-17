using UnrealBuildTool;

public class FicsItNetworksComputer : ModuleRules
{
    public FicsItNetworksComputer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseRTTI = true;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "SML",
                "FactoryGame",
                "FicsItNetworksMisc",
                "FicsItNetworksCircuit",
                "FicsItReflection",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "FicsItLogLibrary",
                "UMG",
                "InputCore",
                "EnhancedInput",
                "GameplayTags",
                "ApplicationCore",
                "HTTP",
            }
        );
    }
}