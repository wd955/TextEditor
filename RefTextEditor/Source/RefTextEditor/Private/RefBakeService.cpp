#include "RefBakeService.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Engine/DataTable.h"
#include "RefTextEditorSettings.h"
#include "MasterReferenceTable.h"
#include "UObject/UnrealType.h"

// Static feature name
const FName IBakeService::FeatureName(TEXT("RefBakeService"));

// Singleton instance used by IBakeService::Get
static FRefBakeService GBakeServiceInstance;

IBakeService& IBakeService::Get()
{
    return GBakeServiceInstance;
}

FString FRefBakeService::BakeText(const FString& In)
{
    return In;
}

int32 FRefBakeService::MeasureBytesUtf8(const FString& In) const
{
    return FTCHARToUTF8(*In).Length();
}

void FRefBakeService::EnsureMirrors(const FMasterRefRow& Ref)
{
    if (!Ref.HardTable)
    {
        return;
    }

    const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
    const FString HardRoot = Settings ? Settings->HardRoot.Path : FString();

    // Ensure the hard table resides in the intended hard directory if one was provided
    const FString HardTablePath = Ref.HardTable->GetOutermost()->GetName();
    if (!Ref.HardFolderOverride.Path.IsEmpty())
    {
        if (!HardTablePath.StartsWith(Ref.HardFolderOverride.Path))
        {
            UE_LOG(LogTemp, Warning, TEXT("Hard table %s is outside designated folder %s"), *HardTablePath, *Ref.HardFolderOverride.Path);
            return;
        }
    }
    else if (!HardRoot.IsEmpty() && !HardTablePath.StartsWith(HardRoot))
    {
        UE_LOG(LogTemp, Warning, TEXT("Hard table %s is outside hard root %s"), *HardTablePath, *HardRoot);
        return;
    }

    UScriptStruct* RowStruct = Ref.HardTable->GetRowStruct();
    if (!RowStruct)
    {
        return;
    }

    const FString SoftRoot = Settings ? Settings->SoftRoot.Path : TEXT("/Game/Editor/Text");
    FString TargetFolder = Ref.SoftFolderOverride.Path;
    if (TargetFolder.IsEmpty() || !TargetFolder.StartsWith(SoftRoot))
    {
        TargetFolder = SoftRoot;
    }

    const FString SoftName = Ref.HardTable->GetName() + TEXT("_Soft");
    const FString PackagePath = TargetFolder / SoftName;

    UPackage* Package = CreatePackage(*PackagePath);
    UDataTable* SoftTable = FindObject<UDataTable>(Package, *SoftName);
    if (!SoftTable)
    {
        SoftTable = NewObject<UDataTable>(Package, *SoftName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(SoftTable);
    }

    SoftTable->RowStruct = RowStruct;
    SoftTable->EmptyTable();

    for (const TPair<FName, uint8*>& Pair : Ref.HardTable->GetRowMap())
    {
        uint8* SourceRow = Pair.Value;
        uint8* Temp = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
        RowStruct->InitializeStruct(Temp);
        RowStruct->CopyScriptStruct(Temp, SourceRow);
        FTableRowBase* RowPtr = reinterpret_cast<FTableRowBase*>(Temp);
        SoftTable->AddRow(Pair.Key, *RowPtr);
        RowStruct->DestroyStruct(Temp);
        FMemory::Free(Temp);
    }

    Package->MarkPackageDirty();
}

void FRefBakeService::SyncEntry(const FMasterRefRow& Ref, const FString& Entry)
{
    // Ensure soft mirrors are available before syncing
    EnsureMirrors(Ref);

    const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
    int32 Limit = Ref.ByteLimitPerCell;
    if (Limit <= 0 && Settings)
    {
        Limit = Settings->ByteLimitPerCell;
    }

    const int32 Bytes = MeasureBytesUtf8(Entry);
    if (Limit > 0 && Bytes > Limit)
    {
        if (Settings && Settings->OverflowPolicy == EOverflowPolicy::ConvertToAssets)
        {
            ConvertEntryToAssets(Entry);
            if (Ref.HardTable)
            {
                SyncTable(Ref.HardTable);
            }
        }
        return;
    }

    // Additional synchronization logic would go here for entries within limits
}

void FRefBakeService::ConvertEntryToAssets(const FString& In)
{
    // Simple placeholder asset creation. In a full implementation this would
    // generate assets on disk and replace table text with references.
    const FString AssetName = FString::Printf(TEXT("RefText_%s"), *FGuid::NewGuid().ToString());
    const FString PackagePath = TEXT("/Game/RefText/Generated/") + AssetName;
    UPackage* Package = CreatePackage(*PackagePath);
    UObject* Asset = NewObject<UObject>(Package, *AssetName, RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Asset);
    Package->MarkPackageDirty();
}

void FRefBakeService::SyncTable(UDataTable* Table)
{
    if (!Table)
    {
        return;
    }

    Table->ForeachRow<FTableRowBase>(TEXT("RefBakeSync"), [&](FTableRowBase& Row)
    {
        // Stub: each row could be processed/baked here
    });
}

bool FRefBakeService::Validate(const FString& In, int32 LimitBytes, FString* OutError) const
{
    const URefTextEditorSettings* Settings = URefTextEditorSettings::Get();
    UDataTable* Master = Settings ? Settings->MasterReferenceTable : nullptr;

    TSet<FString> Visiting;

    TFunction<bool(const FString&)> ValidateImpl = [&](const FString& Text) -> bool
    {
        const int32 Bytes = MeasureBytesUtf8(Text);
        if (LimitBytes > 0 && Bytes > LimitBytes)
        {
            if (OutError)
            {
                *OutError = FString::Printf(TEXT("Text exceeds byte limit (%d/%d)."), Bytes, LimitBytes);
            }
            return false;
        }

        int32 Pos = 0;
        while (Pos < Text.Len())
        {
            const int32 Start = Text.Find(TEXT("{{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Pos);
            if (Start == INDEX_NONE)
            {
                break;
            }
            const int32 End = Text.Find(TEXT("}}"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start + 2);
            if (End == INDEX_NONE)
            {
                break;
            }

            FString TokenString = Text.Mid(Start + 2, End - Start - 2).TrimStartAndEnd();
            TArray<FString> Parts;
            TokenString.ParseIntoArray(Parts, TEXT("."));
            if (Parts.Num() != 3)
            {
                if (OutError)
                {
                    *OutError = FString::Printf(TEXT("Malformed token '%s'."), *TokenString);
                }
                return false;
            }

            const FString& TableName  = Parts[0];
            const FString& RowName    = Parts[1];
            const FString& ColumnName = Parts[2];
            const FString TokenKey = FString::Printf(TEXT("%s.%s.%s"), *TableName, *RowName, *ColumnName);

            if (Visiting.Contains(TokenKey))
            {
                if (OutError)
                {
                    *OutError = FString::Printf(TEXT("Cycle detected at '%s'."), *TokenKey);
                }
                return false;
            }

            FString CellText;
            if (Master)
            {
                UDataTable* TargetTable = nullptr;
                Master->ForeachRow<FMasterRefRow>(TEXT("ValidateLookup"), [&](const FMasterRefRow& Ref)
                {
                    if (!TargetTable && Ref.HardTable && Ref.HardTable->GetName() == TableName)
                    {
                        TargetTable = Ref.HardTable;
                    }
                });

                if (!TargetTable)
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Missing table '%s'."), *TableName);
                    }
                    return false;
                }

                const uint8* const* RowPtrPtr = TargetTable->GetRowMap().Find(FName(*RowName));
                if (!RowPtrPtr)
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Missing row '%s' in table '%s'."), *RowName, *TableName);
                    }
                    return false;
                }
                const uint8* RowPtr = *RowPtrPtr;

                UScriptStruct* RowStruct = TargetTable->GetRowStruct();
                if (!RowStruct)
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Invalid row struct for table '%s'."), *TableName);
                    }
                    return false;
                }

                FProperty* Prop = RowStruct->FindPropertyByName(FName(*ColumnName));
                if (!Prop)
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Missing column '%s' in table '%s'."), *ColumnName, *TableName);
                    }
                    return false;
                }

                const void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(RowPtr);
                if (const FStrProperty* StrProp = CastField<FStrProperty>(Prop))
                {
                    CellText = StrProp->GetPropertyValue(ValuePtr);
                }
                else if (const FTextProperty* TextProp = CastField<FTextProperty>(Prop))
                {
                    CellText = TextProp->GetPropertyValue(ValuePtr).ToString();
                }
                else if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
                {
                    CellText = NameProp->GetPropertyValue(ValuePtr).ToString();
                }
                else
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Unsupported column type for '%s'."), *TokenKey);
                    }
                    return false;
                }

                if (CellText.IsEmpty())
                {
                    if (OutError)
                    {
                        *OutError = FString::Printf(TEXT("Empty value for '%s'."), *TokenKey);
                    }
                    return false;
                }
            }

            Visiting.Add(TokenKey);
            if (!ValidateImpl(CellText))
            {
                return false;
            }
            Visiting.Remove(TokenKey);

            Pos = End + 2;
        }

        return true;
    };

    return ValidateImpl(In);
}

static void RetargetToken(FString& Text, const FString& Table, const FString& OldVal, const FString& NewVal, bool bIsRow)
{
    int32 Pos = 0;
    while (Pos < Text.Len())
    {
        int32 Start = Text.Find(TEXT("{{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Pos);
        if (Start == INDEX_NONE)
        {
            break;
        }
        int32 End = Text.Find(TEXT("}}"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start + 2);
        if (End == INDEX_NONE)
        {
            break;
        }

        FString TokenString = Text.Mid(Start + 2, End - Start - 2).TrimStartAndEnd();
        TArray<FString> Parts;
        TokenString.ParseIntoArray(Parts, TEXT("."));
        if (Parts.Num() == 3 && Parts[0] == Table)
        {
            FString& TargetPart = bIsRow ? Parts[1] : Parts[2];
            if (TargetPart == OldVal)
            {
                TargetPart = NewVal;
                FString NewToken = FString::Printf(TEXT("{{ %s.%s.%s }}"), *Parts[0], *Parts[1], *Parts[2]);
                Text = Text.Left(Start) + NewToken + Text.Mid(End + 2);
                Pos = Start + NewToken.Len();
                continue;
            }
        }

        Pos = End + 2;
    }
}

void FRefBakeService::RetargetRow(FString& Text, const FString& Table, const FString& OldName, const FString& NewName)
{
    RetargetToken(Text, Table, OldName, NewName, true);
}

void FRefBakeService::RetargetColumn(FString& Text, const FString& Table, const FString& OldName, const FString& NewName)
{
    RetargetToken(Text, Table, OldName, NewName, false);
}

