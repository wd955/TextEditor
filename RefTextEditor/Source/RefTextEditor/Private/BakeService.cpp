#include "BakeService.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Engine/DataTable.h"

namespace
{
    /** Simple default implementation. */
    class FDefaultBakeService : public IBakeService
    {
    public:
        virtual FString BakeText(const FString& In) override { return In; }
        virtual int32 MeasureBytesUtf8(const FString& In) const override { return FTCHARToUTF8(*In).Length(); }
        virtual void ConvertEntryToAssets(const FString& In) override { /* stub */ }

        virtual void EnsureMirrors(const FMasterRefRow& Ref) override
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
    };

    FDefaultBakeService GBakeServiceInstance;
}

IBakeService& IBakeService::Get()
{
    return GBakeServiceInstance;
}

