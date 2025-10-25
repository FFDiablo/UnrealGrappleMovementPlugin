using UnrealBuildTool;

public class GrappleMovement : ModuleRules
{
    public GrappleMovement(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "EnhancedInput",
                "GameplayTasks"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CableComponent" // needed for visualising the rope
            }
        );
    }
}