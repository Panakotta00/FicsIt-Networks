using UnrealBuildTool;

public class FicsItReflection : ModuleRules
{
    public FicsItReflection(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Slate",
                "SlateCore",
                "SML",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "FactoryGame"
            }
        );
    }
}