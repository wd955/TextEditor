#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MasterReferenceTable.generated.h"

/**
 * Row describing a single referenced table entry.
 */
USTRUCT(BlueprintType)
struct FMasterRefRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Data table containing hard references for this entry. */
    UPROPERTY(EditAnywhere, Category="Reference")
    TObjectPtr<UDataTable> HardTable = nullptr;

    /** Optional folder to place generated soft assets in. */
    UPROPERTY(EditAnywhere, Category="Reference")
    FDirectoryPath SoftFolderOverride;

    /** Optional folder to place generated hard assets in. */
    UPROPERTY(EditAnywhere, Category="Reference")
    FDirectoryPath HardFolderOverride;

    /** Whether this entry should appear in the editor's left panel. */
    UPROPERTY(EditAnywhere, Category="Reference")
    bool bIncludeInLeftPanel = true;

    /** Maximum allowed bytes for each cell. */
    UPROPERTY(EditAnywhere, Category="Reference")
    int32 ByteLimitPerCell = 0;

    /** Threshold in bytes that will trigger a warning. */
    UPROPERTY(EditAnywhere, Category="Reference")
    int32 WarnThreshold = 0;
};
