#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Text/SlateEditableTextTypes.h"

class SRefTextEditor : public SCompoundWidget
{
public:
        SLATE_BEGIN_ARGS(SRefTextEditor)
                : _OnTextChanged()
        {}
                SLATE_EVENT(FOnTextChanged, OnTextChanged)
        SLATE_END_ARGS()

        void Construct(const FArguments& InArgs);
        virtual bool SupportsKeyboardFocus() const override { return true; }
        virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

        FString GetText() const;

private:
        // UI
        TSharedPtr<class SMultiLineEditableTextBox> TextBox;
        TSharedPtr<class STextBlock>               MisspellCounter;

        // Delegate to notify external widgets about text changes
        FOnTextChanged OnTextChanged;

        struct FMisspelling
        {
                FTextRange Range;
                FString    Word;
        };

        // cached misspelled ranges from the last scan
        TArray<FMisspelling> Misspellings;

        // Spell check is always enabled now
        void ScheduleSpellScan(); // simple immediate call
        void RunSpellScan();

        // context menu helpers
        TSharedPtr<SWidget> OnContextMenuOpening();
        void ReplaceWord(FMisspelling Miss, FString NewWord);

        void AddSelectionToDictionary();
        bool IsWordInCustomDictionary(const FString& Word) const;
};
