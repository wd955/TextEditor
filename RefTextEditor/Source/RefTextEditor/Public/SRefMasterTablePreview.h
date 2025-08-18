#pragma once

#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnRowSelected, int32);

/**
 * Preview widget that shows each line of the master table along with size status.
 */
class SRefMasterTablePreview : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRefMasterTablePreview){}
        SLATE_EVENT(FOnRowSelected, OnRowSelected)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    /** Update the preview from raw text. */
    void SetText(const FString& InText);

private:
    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
    void OnSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

    TArray<TSharedPtr<FString>> Lines;
    TSharedPtr<class SListView<TSharedPtr<FString>>> ListView;
    FOnRowSelected OnRowSelected;
    int32 WarnThreshold = 0;
    int32 ByteLimit = 0;
};

