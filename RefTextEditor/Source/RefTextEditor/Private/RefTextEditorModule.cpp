// RefTextEditorModule.cpp - Minimal editor tab with a 3-panel blank layout

#include "SRefTextEditor.h"
#include "SRefMasterTablePreview.h"
#include "SRefTextPreview.h"
#include "RefTextEditorSettings.h"
#include "MasterReferenceTable.h"
#include "Features/IModularFeatures.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Templates/SharedPointer.h"
#include "Framework/Application/SlateApplication.h"
#include "IBakeService.h"

static const FName RefTextTabName("RefTextEditor");

class FRefTextEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // Register bake service as a modular feature so other modules can access it
        IModularFeatures::Get().RegisterModularFeature(IBakeService::FeatureName, &IBakeService::Get());
        // Register dockable tab
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
            RefTextTabName,
            FOnSpawnTab::CreateRaw(this, &FRefTextEditorModule::SpawnTab)
        )
        .SetDisplayName(NSLOCTEXT("RefText", "TabTitle", "Ref Text Editor"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

        // Add Window menu entry
        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRefTextEditorModule::RegisterMenus));

        // Mirror hard tables into soft copies on startup
        if (const URefTextEditorSettings* Settings = URefTextEditorSettings::Get())
        {
            if (UDataTable* Master = Settings->MasterReferenceTable.LoadSynchronous())
            {
                Master->ForeachRow<FMasterRefRow>(TEXT("StartupMirrors"), [](const FName&, const FMasterRefRow& Row)
                {
                    BakeService::EnsureMirrors(Row);
                });

                Master->OnDataTableChanged().AddLambda([Master]()
                {
                    Master->ForeachRow<FMasterRefRow>(TEXT("ChangedMirrors"), [](const FName&, const FMasterRefRow& Row)
                    {
                        BakeService::EnsureMirrors(Row);
                    });
                });
            }
        }
    }

    virtual void ShutdownModule() override
    {
        // Unregister modular feature on shutdown
        IModularFeatures::Get().UnregisterModularFeature(IBakeService::FeatureName, &IBakeService::Get());
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);

        if (FGlobalTabmanager::Get()->HasTabSpawner(RefTextTabName))
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RefTextTabName);
        }
    }

private:
    void RegisterMenus()
    {
        FToolMenuOwnerScoped OwnerScoped(this);
        if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"))
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntry(
                "OpenRefTextEditor",
                NSLOCTEXT("RefText", "MenuLabel", "Ref Text Editor"),
                NSLOCTEXT("RefText", "MenuTooltip", "Open the Ref Text Editor window"),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([]()
                {
                    FGlobalTabmanager::Get()->TryInvokeTab(RefTextTabName);
                }))
            );
        }
    }

    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs&)
    {
    TSharedRef<SScrollBar> HScrollBar = SNew(SScrollBar).Orientation(Orient_Horizontal);
    TSharedRef<SScrollBar> VScrollBar = SNew(SScrollBar).Orientation(Orient_Vertical);

    TSharedRef<SRefTextEditor> Editor = SNew(SRefTextEditor)
        .ExternalHScrollBar(HScrollBar)
        .ExternalVScrollBar(VScrollBar);

    TWeakPtr<SRefTextEditor> EditorWeak = Editor;

    TSharedRef<SRefTextPreview> Preview = SNew(SRefTextPreview)
        .ExternalHScrollBar(HScrollBar)
        .ExternalVScrollBar(VScrollBar);

    TSharedRef<STextBlock> SizeText = SNew(STextBlock)
        .Text(NSLOCTEXT("RefText", "SizeBar", "Size: 0 KB / 64 KB"));

    TSharedRef<SRefMasterTablePreview> MasterPreview = SNew(SRefMasterTablePreview)
        .OnRowSelected_Lambda([EditorWeak](int32)
        {
            if (TSharedPtr<SRefTextEditor> PinnedEditor = EditorWeak.Pin())
            {
                FSlateApplication::Get().SetKeyboardFocus(PinnedEditor);
            }
        });

    Editor->SetOnTextChanged(FOnTextChanged::CreateLambda(
        [Preview, MasterPreview, SizeText, EditorWeak](const FText& NewText)
        {
            const FString Baked = BakeService::BakeText(NewText.ToString());
            Preview->SetText(Baked);
            MasterPreview->SetText(NewText.ToString());

            const int32 Bytes = BakeService::MeasureBytesUtf8(Baked);

            int32 Limit = 0;
            int32 Warn = 0;
            EOverflowPolicy Policy = EOverflowPolicy::ConvertToAssets;
            if (const URefTextEditorSettings* Settings = URefTextEditorSettings::Get())
            {
                Limit = Settings->ByteLimitPerCell;
                Warn = Settings->WarnThreshold;
                Policy = Settings->OverflowPolicy;
            }
            if (Limit <= 0)
            {
                Limit = 64 * 1024;
            }
            if (Warn <= 0)
            {
                Warn = Limit * 3 / 4;
            }

            FString Error;
            const bool bValid = BakeService::Validate(Baked, Limit, &Error);
            FLinearColor Color = FLinearColor::White;
            if (!bValid || Bytes > Limit)
            {
                Color = FLinearColor::Red;
                if (Policy == EOverflowPolicy::ConvertToAssets)
                {
                    if (TSharedPtr<SRefTextEditor> Pinned = EditorWeak.Pin())
                    {
                        BakeService::ConvertEntryToAssets(Pinned->GetText());
                    }
                }
            }
            else if (Bytes > Warn)
            {
                Color = FLinearColor::Yellow;
            }
            SizeText->SetColorAndOpacity(Color);
            SizeText->SetText(FText::FromString(FString::Printf(TEXT("Size: %.1f KB / %.1f KB"), Bytes / 1024.f, Limit / 1024.f)));
        }));

        TSharedRef<SSplitter> Split =
            SNew(SSplitter)
            .Orientation(Orient_Horizontal)
            // Left
            + SSplitter::Slot().Value(0.28f)
            [
                MasterPreview
            ]
            // Middle
            + SSplitter::Slot().Value(0.44f)
            [
                Editor
            ]
            // Right (with live preview and size bar)
            + SSplitter::Slot().Value(0.28f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(8.f)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Live Preview")))
                ]
                + SVerticalBox::Slot().FillHeight(1.f)
                [
                    Preview
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBorder)
                    .Padding(6.f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SizeText
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f,0.f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString(TEXT("Convert to Asset")))
                                .OnClicked_Lambda([EditorWeak]()
                                {
                                    if (TSharedPtr<SRefTextEditor> PinnedEditor = EditorWeak.Pin())
                                    {
                                        BakeService::ConvertEntryToAssets(PinnedEditor->GetText());
                                    }
                                    return FReply::Handled();
                                })
                        ]
                    ]
                ]
            ];

        return SNew(SDockTab)
               .TabRole(ETabRole::NomadTab)
               [
                   Split
               ];
    }
};

IMPLEMENT_MODULE(FRefTextEditorModule, RefTextEditor)
