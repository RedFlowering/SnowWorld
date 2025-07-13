using UnrealBuildTool;

public class HarmoniaGameService: ModuleRules
{
    public HarmoniaGameService(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
