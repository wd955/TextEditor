#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"

class SRefTextEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRefTextEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

	FString GetText() const;

private:
	// UI
	TSharedPtr<class SMultiLineEditableTextBox> TextBox;
	TSharedPtr<class STextBlock>               MisspellCounter;

	// Spell check is always enabled now
	void ScheduleSpellScan(); // simple immediate call
	void RunSpellScan();

        void AddSelectionToDictionary();
        bool IsWordInCustomDictionary(const FString& Word) const;
};
