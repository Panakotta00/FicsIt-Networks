using UnrealBuildTool;

public class FicsItVisualScriptEditor : ModuleRules
{
    public FicsItVisualScriptEditor(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.Default;

	    PublicDependencyModuleNames.AddRange(
		    new string[]
		    {
			    "Core",
			    "UMG",
			    "Slate",
			    "SlateCore",
		    }
	    );

	    PrivateDependencyModuleNames.AddRange(
		    new string[]
		    {
			    "CoreUObject",
			    "Engine",
			    "InputCore",
			    "ApplicationCore",
		    }
	    );
    }
}
