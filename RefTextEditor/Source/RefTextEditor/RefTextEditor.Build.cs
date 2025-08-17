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
            // The Windows spell checking COM interfaces live in SpellCheck.dll.
            // They can be loaded dynamically via CoCreateInstance, so only the
            // standard COM system libraries are required at link time.  Linking
            // against the import library SpellCheck.lib may fail when the SDK
            // does not provide it, so omit it here.
            PublicSystemLibraries.AddRange(new[] { "Ole32.lib", "OleAut32.lib" });
        }
    }
}
