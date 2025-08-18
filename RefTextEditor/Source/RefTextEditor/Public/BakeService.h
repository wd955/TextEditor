#pragma once

#include "CoreMinimal.h"
#include "MasterReferenceTable.h"

/**
 * Service interface responsible for baking and table mirroring tasks.
 */
class IBakeService
{
public:
    virtual ~IBakeService() = default;

    /** Bake arbitrary text into the desired output format. */
    virtual FString BakeText(const FString& In) = 0;

    /** Measure the UTF-8 byte size of the provided string. */
    virtual int32 MeasureBytesUtf8(const FString& In) const = 0;

    /** Convert a string entry into assets. */
    virtual void ConvertEntryToAssets(const FString& In) = 0;

    /** Ensure a soft mirror table exists for the supplied reference. */
    virtual void EnsureMirrors(const FMasterRefRow& Ref) = 0;

    /** Access the current bake service implementation. */
    static IBakeService& Get();
};

/** Convenience wrapper functions. */
namespace BakeService
{
    inline FString BakeText(const FString& In) { return IBakeService::Get().BakeText(In); }
    inline int32 MeasureBytesUtf8(const FString& In) { return IBakeService::Get().MeasureBytesUtf8(In); }
    inline void ConvertEntryToAssets(const FString& In) { IBakeService::Get().ConvertEntryToAssets(In); }
    inline void EnsureMirrors(const FMasterRefRow& Ref) { IBakeService::Get().EnsureMirrors(Ref); }
}

