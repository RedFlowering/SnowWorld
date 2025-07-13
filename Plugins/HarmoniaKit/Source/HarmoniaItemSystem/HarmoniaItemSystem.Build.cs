using UnrealBuildTool;

public class HarmoniaItemSystem: ModuleRules
{
    public HarmoniaItemSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
