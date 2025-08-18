#include "SRefMasterTablePreview.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "BakeService.h"

namespace
{
    // Thresholds for size warnings in bytes
    constexpr int32 YellowThreshold = 48;
    constexpr int32 RedThreshold = 60;
}

void SRefMasterTablePreview::Construct(const FArguments& InArgs)
{
    OnRowSelected = InArgs._OnRowSelected;

    ChildSlot
    [
        SAssignNew(ListView, SListView<TSharedPtr<FString>>)
            .ListItemsSource(&Lines)
            .OnGenerateRow(this, &SRefMasterTablePreview::OnGenerateRow)
            .OnSelectionChanged(this, &SRefMasterTablePreview::OnSelectionChanged)
    ];
}

void SRefMasterTablePreview::SetText(const FString& InText)
{
    Lines.Reset();
    TArray<FString> RawLines;
    InText.ParseIntoArrayLines(RawLines);
    for (FString& L : RawLines)
    {
        Lines.Add(MakeShared<FString>(MoveTemp(L)));
    }
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SRefMasterTablePreview::OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    int32 Bytes = BakeService::MeasureBytesUtf8(*Item);

    FLinearColor Color = FLinearColor::Gray;
    if (Bytes > RedThreshold)
    {
        Color = FLinearColor::Red;
    }
    else if (Bytes > YellowThreshold)
    {
        Color = FLinearColor::Yellow;
    }

    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
    [
        SNew(STextBlock)
            .Text(FText::FromString(*Item))
            .ColorAndOpacity(Color)
    ];
}

void SRefMasterTablePreview::OnSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
    if (SelectInfo != ESelectInfo::OnMouseClick)
    {
        return;
    }
    if (!Item.IsValid())
    {
        return;
    }
    if (OnRowSelected.IsBound())
    {
        int32 Index = Lines.IndexOfByKey(Item);
        OnRowSelected.Execute(Index);
    }
}

