// RefTextEditorModule.cpp - Minimal editor tab with a 3-panel blank layout

#include "SRefTextEditor.h"
#include "SRefMasterTablePreview.h"
#include "SRefTextPreview.h"
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
#include "BakeService.h"

static const FName RefTextTabName("RefTextEditor");

class FRefTextEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
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
    }

    virtual void ShutdownModule() override
    {
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

    TSharedPtr<SRefTextPreview> Preview;
    TSharedPtr<SRefMasterTablePreview> MasterPreview;
    TSharedPtr<STextBlock> SizeText;

    TSharedRef<SRefTextEditor> Editor = SNew(SRefTextEditor)
        .HScrollBar(HScrollBar)
        .VScrollBar(VScrollBar)
        .OnTextChanged_Lambda([
            &Preview,
            &MasterPreview,
            &SizeText](const FText& NewText)
        {
            const FString Baked = BakeService::BakeText(NewText.ToString());
            if (Preview.IsValid())
            {
                Preview->SetText(Baked);
            }
            if (MasterPreview.IsValid())
            {
                MasterPreview->SetText(NewText.ToString());
            }
            if (SizeText.IsValid())
            {
                const int32 Bytes = BakeService::MeasureBytesUtf8(Baked);
                const int32 Limit = 64 * 1024;
                FLinearColor Color = FLinearColor::White;
                if (Bytes > 60 * 1024)
                {
                    Color = FLinearColor::Red;
                }
                else if (Bytes > 48 * 1024)
                {
                    Color = FLinearColor::Yellow;
                }
                SizeText->SetColorAndOpacity(Color);
                SizeText->SetText(FText::FromString(FString::Printf(TEXT("Size: %.1f KB / 64 KB"), Bytes / 1024.f)));
            }
        });

    SAssignNew(Preview, SRefTextPreview)
        .ExternalHScrollBar(HScrollBar)
        .ExternalVScrollBar(VScrollBar);

    SAssignNew(MasterPreview, SRefMasterTablePreview)
        .OnRowSelected_Lambda([EditorWeak = TWeakPtr<SRefTextEditor>(Editor)](int32)
        {
            if (TSharedPtr<SRefTextEditor> PinnedEditor = EditorWeak.Pin())
            {
                FSlateApplication::Get().SetKeyboardFocus(PinnedEditor);
            }
        });

        TSharedRef<SSplitter> Split =
            SNew(SSplitter)
            .Orientation(Orient_Horizontal)
            // Left
            + SSplitter::Slot().Value(0.28f)
            [
                MasterPreview.ToSharedRef()
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
                    Preview.ToSharedRef()
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBorder)
                    .Padding(6.f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SAssignNew(SizeText, STextBlock)
                                .Text(NSLOCTEXT("RefText", "SizeBar", "Size: 0 KB / 64 KB"))
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f,0.f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString(TEXT("Convert to Asset")))
                                .OnClicked_Lambda([EditorWeak = TWeakPtr<SRefTextEditor>(Editor)]()
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
