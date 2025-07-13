using UnrealBuildTool;

public class HarmoniaWorldObjectSystem: ModuleRules
{
    public HarmoniaWorldObjectSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
