#pragma once

#include "Widgets/SCompoundWidget.h"

class SScrollBar;

class SRefTextPreview : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRefTextPreview){}
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalHScrollBar)
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalVScrollBar)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    /** Set baked text to display */
    void SetText(const FString& InText);

private:
    TSharedPtr<class SMultiLineEditableTextBox> TextBox;
};
