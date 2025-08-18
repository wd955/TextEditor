#pragma once

#include "IBakeService.h"

/** Concrete implementation of the bake service used by the RefTextEditor. */
class FRefBakeService : public IBakeService
{
public:
    virtual FString BakeText(const FString& In) override;
    virtual int32 MeasureBytesUtf8(const FString& In) const override;
    virtual void EnsureMirrors(const FMasterRefRow& Ref) override;
    virtual void SyncEntry(const FMasterRefRow& Ref, const FString& Entry) override;
    virtual void ConvertEntryToAssets(const FString& In) override;
    virtual void SyncTable(UDataTable* Table) override;
    virtual bool Validate(const FString& In, int32 LimitBytes, FString* OutError) const override;
    virtual void RetargetRow(FString& Text, const FString& Table, const FString& OldName, const FString& NewName) override;
    virtual void RetargetColumn(FString& Text, const FString& Table, const FString& OldName, const FString& NewName) override;
};

