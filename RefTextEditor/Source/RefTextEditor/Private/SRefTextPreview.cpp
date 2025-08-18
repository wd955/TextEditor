#include "SRefTextPreview.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBar.h"

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
    }
}
