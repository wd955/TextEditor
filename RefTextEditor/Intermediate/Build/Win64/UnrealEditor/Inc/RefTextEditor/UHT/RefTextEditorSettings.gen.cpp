// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "RefTextEditor/Public/RefTextEditorSettings.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeRefTextEditorSettings() {}

// Begin Cross Module References
DEVELOPERSETTINGS_API UClass* Z_Construct_UClass_UDeveloperSettings();
REFTEXTEDITOR_API UClass* Z_Construct_UClass_URefTextEditorSettings();
REFTEXTEDITOR_API UClass* Z_Construct_UClass_URefTextEditorSettings_NoRegister();
UPackage* Z_Construct_UPackage__Script_RefTextEditor();
// End Cross Module References

// Begin Class URefTextEditorSettings
void URefTextEditorSettings::StaticRegisterNativesURefTextEditorSettings()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(URefTextEditorSettings);
UClass* Z_Construct_UClass_URefTextEditorSettings_NoRegister()
{
	return URefTextEditorSettings::StaticClass();
}
struct Z_Construct_UClass_URefTextEditorSettings_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Project-level settings for Ref Text Editor.\n * Appears in Project Settings \xe2\x86\x92 Plugins \xe2\x86\x92 Ref Text Editor\n */" },
#endif
		{ "DisplayName", "Ref Text Editor" },
		{ "IncludePath", "RefTextEditorSettings.h" },
		{ "ModuleRelativePath", "Public/RefTextEditorSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Project-level settings for Ref Text Editor.\nAppears in Project Settings \xe2\x86\x92 Plugins \xe2\x86\x92 Ref Text Editor" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CustomDictionary_MetaData[] = {
		{ "Category", "Spell Check" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Words treated as correctly spelled (case-insensitive)\n" },
#endif
		{ "ModuleRelativePath", "Public/RefTextEditorSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Words treated as correctly spelled (case-insensitive)" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_CustomDictionary_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_CustomDictionary;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<URefTextEditorSettings>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UClass_URefTextEditorSettings_Statics::NewProp_CustomDictionary_Inner = { "CustomDictionary", nullptr, (EPropertyFlags)0x0000000000004000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UClass_URefTextEditorSettings_Statics::NewProp_CustomDictionary = { "CustomDictionary", nullptr, (EPropertyFlags)0x0010000000004001, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(URefTextEditorSettings, CustomDictionary), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CustomDictionary_MetaData), NewProp_CustomDictionary_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_URefTextEditorSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_URefTextEditorSettings_Statics::NewProp_CustomDictionary_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_URefTextEditorSettings_Statics::NewProp_CustomDictionary,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_URefTextEditorSettings_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_URefTextEditorSettings_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UDeveloperSettings,
	(UObject* (*)())Z_Construct_UPackage__Script_RefTextEditor,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_URefTextEditorSettings_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_URefTextEditorSettings_Statics::ClassParams = {
	&URefTextEditorSettings::StaticClass,
	"EditorPerProjectUserSettings",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_URefTextEditorSettings_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_URefTextEditorSettings_Statics::PropPointers),
	0,
	0x000000A6u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_URefTextEditorSettings_Statics::Class_MetaDataParams), Z_Construct_UClass_URefTextEditorSettings_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_URefTextEditorSettings()
{
	if (!Z_Registration_Info_UClass_URefTextEditorSettings.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_URefTextEditorSettings.OuterSingleton, Z_Construct_UClass_URefTextEditorSettings_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_URefTextEditorSettings.OuterSingleton;
}
template<> REFTEXTEDITOR_API UClass* StaticClass<URefTextEditorSettings>()
{
	return URefTextEditorSettings::StaticClass();
}
URefTextEditorSettings::URefTextEditorSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(URefTextEditorSettings);
URefTextEditorSettings::~URefTextEditorSettings() {}
// End Class URefTextEditorSettings

// Begin Registration
struct Z_CompiledInDeferFile_FID_TextEditor_Plugins_RefTextEditor_Source_RefTextEditor_Public_RefTextEditorSettings_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_URefTextEditorSettings, URefTextEditorSettings::StaticClass, TEXT("URefTextEditorSettings"), &Z_Registration_Info_UClass_URefTextEditorSettings, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(URefTextEditorSettings), 3144646241U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TextEditor_Plugins_RefTextEditor_Source_RefTextEditor_Public_RefTextEditorSettings_h_696025260(TEXT("/Script/RefTextEditor"),
	Z_CompiledInDeferFile_FID_TextEditor_Plugins_RefTextEditor_Source_RefTextEditor_Public_RefTextEditorSettings_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_TextEditor_Plugins_RefTextEditor_Source_RefTextEditor_Public_RefTextEditorSettings_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
