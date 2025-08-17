#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RefTextEditorSettings.generated.h"

/**
 * Project-level settings for Ref Text Editor.
 * Appears in Project Settings → Plugins → Ref Text Editor
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Ref Text Editor"))
class URefTextEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// Words treated as correctly spelled (case-insensitive)
	UPROPERTY(EditAnywhere, Config, Category = "Spell Check")
	TArray<FString> CustomDictionary;

	// Utility to access current settings
	static const URefTextEditorSettings* Get() { return GetDefault<URefTextEditorSettings>(); }
	static URefTextEditorSettings* GetMutable() { return GetMutableDefault<URefTextEditorSettings>(); }
};
