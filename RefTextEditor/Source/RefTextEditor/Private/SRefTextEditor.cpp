#include "SRefTextEditor.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Spell/SpellChecker.h"
#include "RefTextEditorSettings.h"
#include "Editor.h"

void SRefTextEditor::Construct(const FArguments&)
{
	ChildSlot
		[
			SNew(SVerticalBox)

                                // Small status row with misspell counter and options menu
                                + SVerticalBox::Slot().AutoHeight().Padding(4)
                                [
                                        SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                        [
                                                SAssignNew(MisspellCounter, STextBlock)
                                                        .Text(FText::FromString(TEXT("Misspellings: 0")))
                                        ]
                                        + SHorizontalBox::Slot().AutoWidth().Padding(8,0,0,0).VAlign(VAlign_Center)
                                        [
                                                SNew(SButton)
                                                        .Text(FText::FromString(TEXT("⋯")))
                                                        .OnClicked_Lambda([this]()
                                                        {
                                                                FMenuBuilder Menu(true, nullptr);
                                                                Menu.AddMenuEntry(
                                                                        FText::FromString(TEXT("Add selection to dictionary")),
                                                                        FText::FromString(TEXT("Treat the selected word as correct.")),
                                                                        FSlateIcon(),
                                                                        FUIAction(FExecuteAction::CreateSP(this, &SRefTextEditor::AddSelectionToDictionary))
                                                                );
                                                                FSlateApplication::Get().PushMenu(
                                                                        AsShared(),
                                                                        FWidgetPath(),
                                                                        Menu.MakeWidget(),
                                                                        FSlateApplication::Get().GetCursorPos(),
                                                                        FPopupTransitionEffect::ContextMenu
                                                                );
                                                                return FReply::Handled();
                                                        })
                                        ]
                                ]

				// Editor
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
				[
					SNew(SBorder)
						.OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent&)
							{
								if (TextBox.IsValid())
								{
									FSlateApplication::Get().SetKeyboardFocus(TextBox, EFocusCause::Mouse);
								}
								return FReply::Handled();
							})
						[
							SAssignNew(TextBox, SMultiLineEditableTextBox)
								.IsReadOnly(false)
								.AlwaysShowScrollbars(true)
								.AutoWrapText(true)
								.HintText(FText::FromString(TEXT("Type here…")))
                                                                .OnTextChanged_Lambda([this](const FText&) { ScheduleSpellScan(); })
                                                ]
                                ]
                ];

	ScheduleSpellScan();
}

FReply SRefTextEditor::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	if (TextBox.IsValid())
	{
		return FReply::Handled().SetUserFocus(TextBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
	return FReply::Unhandled();
}

FString SRefTextEditor::GetText() const
{
	return TextBox.IsValid() ? TextBox->GetText().ToString() : FString();
}

void SRefTextEditor::ScheduleSpellScan()
{
	// Simple immediate scan (no timers for now)
	RunSpellScan();
}

bool SRefTextEditor::IsWordInCustomDictionary(const FString& Word) const
{
	const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
	if (!Settings) return false;

	const FString Lower = Word.ToLower();
	for (const FString& W : Settings->CustomDictionary)
	{
		if (Lower.Equals(W.ToLower()))
		{
			return true;
		}
	}
	return false;
}

void SRefTextEditor::RunSpellScan()
{
	int32 MissCount = 0;
	const FString Text = GetText();
	if (!Text.IsEmpty())
	{
		TSharedPtr<ISpellChecker> SC = CreateSpellChecker();
		auto IsWord = [](TCHAR C) { return FChar::IsAlpha(C) || C == '\'' || C == '-'; };

		int32 i = 0, N = Text.Len();
		while (i < N)
		{
			while (i < N && !IsWord(Text[i])) ++i;
			const int32 Start = i;
			while (i < N && IsWord(Text[i])) ++i;
			const int32 End = i;

			if (End > Start)
			{
				const FString Word = Text.Mid(Start, End - Start);
				// First: custom dictionary
				if (IsWordInCustomDictionary(Word))
				{
					continue; // treat as correct
				}
				// Then: platform spell checker (dummy or real)
				if (SC.IsValid() && !SC->Check(Word))
				{
					++MissCount;
				}
			}
		}
	}

        if (MisspellCounter.IsValid())
        {
                MisspellCounter->SetText(FText::FromString(FString::Printf(TEXT("Misspellings: %d"), MissCount)));
        }
}

void SRefTextEditor::AddSelectionToDictionary()
{
	if (!TextBox.IsValid()) return;

	// Use the current selection as the word to add
	const FText Selected = TextBox->GetSelectedText();
	const FString SelectedStr = Selected.ToString().TrimStartAndEnd();

	// Validate: single "wordish" token
	if (SelectedStr.IsEmpty())
	{
		return;
	}
	auto IsWordish = [](const FString& S)
		{
			for (TCHAR C : S) { if (!(FChar::IsAlpha(C) || C == '\'' || C == '-')) return false; }
			return true;
		};
	if (!IsWordish(SelectedStr))
	{
		return;
	}

	// Add to settings (case-insensitive; store lower for consistency)
	URefTextEditorSettings* Settings = URefTextEditorSettings::GetMutable();
	if (!Settings) return;

	const FString Lower = SelectedStr.ToLower();
	bool bAlready = false;
	for (const FString& W : Settings->CustomDictionary)
	{
		if (Lower.Equals(W.ToLower())) { bAlready = true; break; }
	}
	if (!bAlready)
	{
		Settings->CustomDictionary.Add(Lower);
		Settings->SaveConfig(); // persist to EditorPerProjectUserSettings
	}

	// Re-scan to update the counter
	ScheduleSpellScan();
}
