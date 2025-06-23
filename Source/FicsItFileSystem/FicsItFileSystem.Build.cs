using UnrealBuildTool;

public class FicsItFileSystem : ModuleRules
{
    public FicsItFileSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
	    CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;
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
                "FicsItLogLibrary"
            }
        );
    }
}