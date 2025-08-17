using UnrealBuildTool;

public class RefTextEditor : ModuleRules
{
    public RefTextEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "Slate",
            "SlateCore",
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject",
            "Engine",
            "ApplicationCore",
            "InputCore",
            "UnrealEd",
            "LevelEditor",
            "ToolMenus",
            "AssetRegistry",
            "AssetTools",
            "Projects",
            "Json",
            "JsonUtilities",
            "DeveloperSettings"   // <-- ADD THIS
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("REFTEXT_WINDOWS_SPELL=1");
            PublicSystemLibraries.AddRange(new[] { "Ole32.lib", "OleAut32.lib", "SpellCheck.lib" });
        }
    }
}
