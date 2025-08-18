#include "SRefTextEditor.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Text/TextLayout.h"
#include "InputCoreTypes.h"

#include "Spell/SpellChecker.h"
#include "RefTextEditorSettings.h"
#include "MasterReferenceTable.h"
#include "Engine/DataTable.h"
#include "UObject/UnrealType.h"
#include "Editor.h"
#include "Dom/JsonValue.h"

void SRefTextEditor::Construct(const FArguments& InArgs)
{
		OnTextChanged = InArgs._OnTextChanged;

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
                                                                                                                .Text(FText::FromString(TEXT("...")))
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

								// Editor only (preview handled externally)
								+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
								[
										SNew(SBorder)
												.OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent& MouseEvent)
														{
																if (TextBox.IsValid())
																{
																		FSlateApplication::Get().SetKeyboardFocus(TextBox, EFocusCause::Mouse);
																		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
																		{
																				return FReply::Handled();
																		}
																}
																return FReply::Unhandled();
														})
												[
														SAssignNew(TextBox, SMultiLineEditableTextBox)
																.IsReadOnly(false)
																.AlwaysShowScrollbars(true)
																.AutoWrapText(true)
																.HintText(FText::FromString(TEXT("Type here…")))
																.HScrollBar(InArgs._ExternalHScrollBar)
																.VScrollBar(InArgs._ExternalVScrollBar)
																.OnTextChanged_Lambda([this](const FText& NewText)
																		{
																				OnTextChanged.ExecuteIfBound(NewText);
																				ScheduleSpellScan();
																		})
																.OnContextMenuOpening(FOnContextMenuOpening::CreateSP(this, &SRefTextEditor::OnContextMenuOpening))
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

void SRefTextEditor::SetOnTextChanged(FOnTextChanged InHandler)
{
        OnTextChanged = InHandler;
}

FString SRefTextEditor::GetText() const
{
                return TextBox.IsValid() ? TextBox->GetText().ToString() : FString();
}

FString SRefTextEditor::Serialize() const
{
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		const FString PlainText = GetText();
		Root->SetStringField(TEXT("text"), PlainText);

		// Parse tokens directly from the plain text using the {{ table.row.column }} pattern
		TArray<TSharedPtr<FJsonValue>> TokenArray;
		int32 SearchPos = 0;
		while (SearchPos < PlainText.Len())
		{
				int32 Start = PlainText.Find(TEXT("{{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos);
				if (Start == INDEX_NONE) break;
				int32 End = PlainText.Find(TEXT("}}"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
				if (End == INDEX_NONE) break;

				const FString Inside = PlainText.Mid(Start + 2, End - Start - 2).TrimStartAndEnd();
				TArray<FString> Parts;
				Inside.ParseIntoArray(Parts, TEXT("."));
				if (Parts.Num() >= 3)
				{
						FTokenMeta Meta;
						Meta.Table = Parts[0];
						Meta.Row = Parts[1];
						Meta.Column = Parts[2];

						TSharedRef<FJsonObject> TokenObj = MakeShared<FJsonObject>();
						TokenObj->SetNumberField(TEXT("start"), Start);
						TokenObj->SetNumberField(TEXT("end"), End + 2);
						TokenObj->SetStringField(TEXT("meta"), Meta.ToJson());
						TokenArray.Add(MakeShared<FJsonValueObject>(TokenObj));
				}
				SearchPos = End + 2;
		}

		Root->SetArrayField(TEXT("tokens"), TokenArray);

		FString Out;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
		FJsonSerializer::Serialize(Root, Writer);
		return Out;
}

void SRefTextEditor::Deserialize(const FString& InSerialized)
{
		TSharedPtr<FJsonObject> Root;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InSerialized);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
				return;
		}

		FString PlainText;
		Root->TryGetStringField(TEXT("text"), PlainText);
		if (TextBox.IsValid())
		{
				TextBox->SetText(FText::FromString(PlainText));
		}
}

void SRefTextEditor::InsertToken(const FTokenMeta& Meta)
{
		if (!TextBox.IsValid()) return;

		const FString Display = Meta.ToDisplayString();
		TextBox->InsertTextAtCursor(Display);
}

FReply SRefTextEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
		if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::SpaceBar)
		{
				CachedSuggestions = BuildTokenSuggestions();
				ShowSuggestions(CachedSuggestions);
				return FReply::Handled();
		}
		return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
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

TArray<FString> SRefTextEditor::BuildTokenSuggestions() const
{
		TArray<FString> Suggestions;
		const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
		if (!Settings) return Suggestions;

		if (UDataTable* Master = Settings->MasterReferenceTable.LoadSynchronous())
		{
				Master->ForeachRow<FMasterRefRow>(TEXT("TokenSuggestions"), [&Suggestions](const FName& RowName, const FMasterRefRow& Row)
				{
						(void)RowName;
						if (!Row.HardTable) return;
						const FString TableName = Row.HardTable->GetName();
						for (const auto& Pair : Row.HardTable->GetRowMap())
						{
								const FString RowNameStr = Pair.Key.ToString();
								const UScriptStruct* Struct = Row.HardTable->GetRowStruct();
								for (TFieldIterator<FProperty> It(Struct); It; ++It)
								{
										const FString Column = It->GetName();
										Suggestions.Add(FString::Printf(TEXT("%s.%s.%s"), *TableName, *RowNameStr, *Column));
								}
						}
				});
		}

		return Suggestions;
}

void SRefTextEditor::ShowSuggestions(const TArray<FString>& InSuggestions)
{
		FMenuBuilder Menu(true, nullptr);
		for (const FString& Token : InSuggestions)
		{
				Menu.AddMenuEntry(
						FText::FromString(Token),
						FText::GetEmpty(),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda([this, Token]() { HandleSuggestionChosen(Token); }))
				);
		}

		FSlateApplication::Get().PushMenu(
				AsShared(),
				FWidgetPath(),
				Menu.MakeWidget(),
				FSlateApplication::Get().GetCursorPos(),
				FPopupTransitionEffect::ContextMenu
		);
}

void SRefTextEditor::HandleSuggestionChosen(const FString& TokenString)
{
		FTokenMeta Meta;
		TArray<FString> Parts;
		TokenString.ParseIntoArray(Parts, TEXT("."));
		if (Parts.Num() >= 1) Meta.Table = Parts[0];
		if (Parts.Num() >= 2) Meta.Row	 = Parts[1];
		if (Parts.Num() >= 3) Meta.Column= Parts[2];
		InsertToken(Meta);
}

void SRefTextEditor::RunSpellScan()
{
		Misspellings.Reset();
		int32 MissCount = 0;
		const FString Text = GetText();
		if (!Text.IsEmpty())
		{
				const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
				TSharedPtr<IRefSpellChecker> SC = CreateSpellChecker(Settings ? Settings->DefaultLanguage : FString());
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
										FMisspelling Miss;
										Miss.Range = FTextRange(Start, End);
										Miss.Word  = Word;
										Misspellings.Add(Miss);
								}
						}
				}
		}

		if (MisspellCounter.IsValid())
		{
				MisspellCounter->SetText(FText::FromString(FString::Printf(TEXT("Misspellings: %d"), MissCount)));
		}

		// Highlighting API changed in recent engine versions; skip for now
		if (TextBox.IsValid())
		{
				// Intentionally left blank
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

TSharedPtr<SWidget> SRefTextEditor::OnContextMenuOpening()
{
		if (!TextBox.IsValid()) return nullptr;

		const FString Selected = TextBox->GetSelectedText().ToString();
		if (Selected.IsEmpty()) return nullptr; // default menu

		const FMisspelling* Found = nullptr;
		for (const FMisspelling& M : Misspellings)
		{
				if (Selected.Equals(M.Word, ESearchCase::CaseSensitive))
				{
						Found = &M;
						break;
				}
		}

		if (!Found)
		{
				return nullptr; // let default menu appear
		}

		FMenuBuilder Menu(true, nullptr);

		const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
		TSharedPtr<IRefSpellChecker> SC = CreateSpellChecker(Settings ? Settings->DefaultLanguage : FString());
		if (SC.IsValid())
		{
				TArray<FString> Suggestions;
				SC->Suggest(Found->Word, Suggestions);
				for (const FString& Sugg : Suggestions)
				{
					   Menu.AddMenuEntry(
							   FText::FromString(Sugg),
							   FText::FromString(TEXT("Replace with suggestion")),
							   FSlateIcon(),
							   FUIAction(FExecuteAction::CreateSP(this, &SRefTextEditor::ReplaceWord, *Found, Sugg))
					   );
				}
		}

		Menu.AddSeparator();
	   Menu.AddMenuEntry(
			   FText::FromString(TEXT("Add to dictionary")),
			   FText::FromString(TEXT("Treat this word as correct")),
			   FSlateIcon(),
			   FUIAction(FExecuteAction::CreateSP(this, &SRefTextEditor::AddSelectionToDictionary))
	   );

		return Menu.MakeWidget();
}

void SRefTextEditor::ReplaceWord(FMisspelling Miss, FString NewWord)
{
		if (!TextBox.IsValid()) return;

		FString Text = GetText();
		FString NewText = Text.Left(Miss.Range.BeginIndex) + NewWord + Text.Mid(Miss.Range.EndIndex);
		TextBox->SetText(FText::FromString(NewText));
		ScheduleSpellScan();
}
