using UnrealBuildTool;

public class HarmoniaLoadManagerEditor: ModuleRules
{
    public HarmoniaLoadManagerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "HarmoniaLoadManager",
            });
    }
}
