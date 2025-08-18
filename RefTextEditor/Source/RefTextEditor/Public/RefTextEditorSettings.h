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
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Ref Text Editor"))
class URefTextEditorSettings : public UDeveloperSettings
{
        GENERATED_BODY()
public:
	// Words treated as correctly spelled (case-insensitive)
        UPROPERTY(EditAnywhere, Config, Category = "Spell Check")
        TArray<FString> CustomDictionary;

        // Data table containing master reference information
        UPROPERTY(EditAnywhere, Config, Category = "Master Reference")
        TObjectPtr<UDataTable> MasterReferenceTable = nullptr;

	// Utility to access current settings
	static const URefTextEditorSettings* Get() { return GetDefault<URefTextEditorSettings>(); }
	static URefTextEditorSettings* GetMutable() { return GetMutableDefault<URefTextEditorSettings>(); }
};
