#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RefTextEditorSettings.generated.h"

class UDataTable;

/**
 * Project-level settings for Ref Text Editor.
 * Appears in Project Settings → Plugins → Ref Text Editor.
 * Soft tables live under /Game/Editor/Text and are never cooked into builds.
 */
UENUM()
enum class EOverflowPolicy : uint8
{
        ConvertToAssets,
        Error
};

UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Ref Text Editor"))
class URefTextEditorSettings : public UDeveloperSettings
{
        GENERATED_BODY()
public:
        URefTextEditorSettings();

        // Words treated as correctly spelled (case-insensitive)
        UPROPERTY(EditAnywhere, Config, Category = "Spell Check")
        TArray<FString> CustomDictionary;

        // Data table containing master reference information
        UPROPERTY(EditAnywhere, Config, Category = "Master Reference")
        TObjectPtr<UDataTable> MasterReferenceTable = nullptr;

        // Root folder for generated soft assets
        UPROPERTY(EditAnywhere, Config, Category = "Paths")
        FDirectoryPath SoftRoot;

        // Root folder for hard reference tables
        UPROPERTY(EditAnywhere, Config, Category = "Paths")
        FDirectoryPath HardRoot;

        // Threshold in bytes that triggers a warning
        UPROPERTY(EditAnywhere, Config, Category = "Limits")
        int32 WarnThreshold = 0;

        // Maximum allowed bytes for each cell
        UPROPERTY(EditAnywhere, Config, Category = "Limits")
        int32 ByteLimitPerCell = 0;

        // Action to take when text exceeds the byte limit
        UPROPERTY(EditAnywhere, Config, Category = "Limits")
        EOverflowPolicy OverflowPolicy = EOverflowPolicy::ConvertToAssets;

        // Default language used for spell checking
        UPROPERTY(EditAnywhere, Config, Category = "Localization")
        FString DefaultLanguage;

        // Utility to access current settings
        static const URefTextEditorSettings* Get() { return GetDefault<URefTextEditorSettings>(); }
        static URefTextEditorSettings* GetMutable() { return GetMutableDefault<URefTextEditorSettings>(); }
};

inline URefTextEditorSettings::URefTextEditorSettings()
{
        SoftRoot.Path = TEXT("/Game/Editor/Text");
        HardRoot.Path = TEXT("/Game/RefText");
        WarnThreshold = 48;
        ByteLimitPerCell = 60;
        OverflowPolicy = EOverflowPolicy::ConvertToAssets;
        DefaultLanguage = TEXT("en-US");
}
