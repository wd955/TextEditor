#pragma once
#include "Templates/SharedPointer.h"

// Renamed from ISpellChecker to avoid clashing with the Windows
// ISpellChecker COM interface exposed by <spellcheck.h>.
struct IRefSpellChecker : public TSharedFromThis<IRefSpellChecker>
{
    virtual ~IRefSpellChecker() = default;
    virtual bool Check(const FString& Word) = 0;
    virtual void Suggest(const FString& Word, TArray<FString>& OutSuggestions) = 0;
};

TSharedPtr<IRefSpellChecker> CreateSpellChecker();
