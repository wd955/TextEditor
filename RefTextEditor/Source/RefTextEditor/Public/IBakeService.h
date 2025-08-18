#pragma once

#include "CoreMinimal.h"
#include "MasterReferenceTable.h"
#include "Engine/DataTable.h"
#include "Modules/ModularFeature.h"

/**
 * Interface describing the bake/text syncing service used by the editor.
 */
class IBakeService : public IModularFeature
{
public:
    /** Name used when registering as a modular feature. */
    static const FName FeatureName;

    virtual ~IBakeService() = default;

    /** Bake arbitrary text into the desired output format. */
    virtual FString BakeText(const FString& In) = 0;

    /** Measure the UTF-8 byte size of the provided string. */
    virtual int32 MeasureBytesUtf8(const FString& In) const = 0;

    /** Ensure a soft mirror table exists for the supplied reference. */
    virtual void EnsureMirrors(const FMasterRefRow& Ref) = 0;

    /** Sync a single entry for the given reference row. */
    virtual void SyncEntry(const FMasterRefRow& Ref, const FString& Entry) = 0;

    /** Convert the provided text entry into one or more assets. */
    virtual void ConvertEntryToAssets(const FString& In) = 0;

    /** Sync an entire hard table to its soft mirror. */
    virtual void SyncTable(UDataTable* Table) = 0;

    /** Validate the supplied text against limits. */
    virtual bool Validate(const FString& In, int32 LimitBytes, FString* OutError) const = 0;

    /** Access the currently registered bake service implementation. */
    static IBakeService& Get();
};

/** Convenience wrapper functions. */
namespace BakeService
{
    inline FString BakeText(const FString& In) { return IBakeService::Get().BakeText(In); }
    inline int32 MeasureBytesUtf8(const FString& In) { return IBakeService::Get().MeasureBytesUtf8(In); }
    inline void EnsureMirrors(const FMasterRefRow& Ref) { IBakeService::Get().EnsureMirrors(Ref); }
    inline void SyncEntry(const FMasterRefRow& Ref, const FString& Entry) { IBakeService::Get().SyncEntry(Ref, Entry); }
    inline void ConvertEntryToAssets(const FString& In) { IBakeService::Get().ConvertEntryToAssets(In); }
    inline void SyncTable(UDataTable* Table) { IBakeService::Get().SyncTable(Table); }
    inline bool Validate(const FString& In, int32 LimitBytes, FString* OutError = nullptr) { return IBakeService::Get().Validate(In, LimitBytes, OutError); }
}

