#include "SRefMasterTablePreview.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "IBakeService.h"
#include "RefTextEditorSettings.h"
#include "MasterReferenceTable.h"
#include "Engine/DataTable.h"

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
    WarnThreshold = 0;
    ByteLimit = 0;
    if (const URefTextEditorSettings* Settings = URefTextEditorSettings::Get())
    {
        if (UDataTable* Master = Settings->MasterReferenceTable)
        {
            TArray<FMasterRefRow*> Rows;
            Master->GetAllRows<FMasterRefRow>(TEXT("PreviewLimits"), Rows);
            if (Rows.Num() > 0)
            {
                ByteLimit = Rows[0]->ByteLimitPerCell;
                WarnThreshold = Rows[0]->WarnThreshold;
            }
        }
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
    const int32 Limit = ByteLimit > 0 ? ByteLimit : 60;
    const int32 Warn = WarnThreshold > 0 ? WarnThreshold : 48;
    if (Bytes > Limit)
    {
        Color = FLinearColor::Red;
    }
    else if (Bytes > Warn)
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

