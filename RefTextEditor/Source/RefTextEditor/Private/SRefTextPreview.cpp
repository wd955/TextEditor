#include "SRefTextPreview.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "IBakeService.h"

void SRefTextPreview::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SAssignNew(TextBox, SMultiLineEditableTextBox)
            .IsReadOnly(true)
            .AlwaysShowScrollbars(true)
            .AutoWrapText(true)
            .HScrollBar(InArgs._ExternalHScrollBar)
            .VScrollBar(InArgs._ExternalVScrollBar)
    ];
}

void SRefTextPreview::SetText(const FString& InText)
{
    if (TextBox.IsValid())
    {
        TextBox->SetText(FText::FromString(InText));
        FString Error;
        const bool bValid = BakeService::Validate(InText, 0, &Error);
        TextBox->SetForegroundColor(bValid ? FLinearColor::White : FLinearColor::Red);
    }
}
