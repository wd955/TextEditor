#include "RefBakeService.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Engine/DataTable.h"

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

    UScriptStruct* RowStruct = Ref.HardTable->GetRowStruct();
    if (!RowStruct)
    {
        return;
    }

    FString TargetFolder = Ref.SoftFolderOverride.Path;
    if (TargetFolder.IsEmpty())
    {
        TargetFolder = TEXT("/Game/RefText/Mirrors");
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

    const int32 Bytes = MeasureBytesUtf8(Entry);
    if (Ref.ByteLimitPerCell > 0 && Bytes > Ref.ByteLimitPerCell)
    {
        // Entry exceeds the limit, convert it into external assets
        ConvertEntryToAssets(Entry);

        // After conversion, update mirror tables so they point to the new asset
        if (Ref.HardTable)
        {
            SyncTable(Ref.HardTable);
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
    const int32 Bytes = MeasureBytesUtf8(In);
    if (LimitBytes > 0 && Bytes > LimitBytes)
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Text exceeds byte limit (%d/%d)."), Bytes, LimitBytes);
        }
        return false;
    }
    return true;
}

