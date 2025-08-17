// RefTextEditorModule.cpp - Minimal editor tab with a 3-panel blank layout

#include "SRefTextEditor.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

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
        auto MakePanel = [](const TCHAR* Label)
        {
            return SNew(SBorder)
                   .Padding(8.f)
                   [
                       SNew(STextBlock).Text(FText::FromString(Label))
                   ];
        };

        TSharedRef<SSplitter> Split =
            SNew(SSplitter)
            .Orientation(Orient_Horizontal)
            // Left
            + SSplitter::Slot().Value(0.28f)
            [
                MakePanel(TEXT("Left Panel — Master Table Preview"))
            ]
            // Middle
            + SSplitter::Slot().Value(0.44f)
            [
                SNew(SRefTextEditor)
            ]
            // Right (with size bar placeholder)
            + SSplitter::Slot().Value(0.28f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().FillHeight(1.f)
                [
                    MakePanel(TEXT("Right Panel — Live Preview"))
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBorder)
                    .Padding(6.f)
                    [
                        SNew(STextBlock).Text(NSLOCTEXT("RefText", "SizeBar", "Size: 0 KB / 64 KB"))
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
