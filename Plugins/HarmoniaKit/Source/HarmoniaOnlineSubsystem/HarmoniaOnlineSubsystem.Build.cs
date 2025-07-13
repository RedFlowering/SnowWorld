using UnrealBuildTool;

public class HarmoniaOnlineSubsystem: ModuleRules
{
    public HarmoniaOnlineSubsystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
