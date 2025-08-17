#pragma once
#include "Templates/SharedPointer.h"

struct ISpellChecker : public TSharedFromThis<ISpellChecker>
{
    virtual ~ISpellChecker() = default;
    virtual bool Check(const FString& Word) = 0;
    virtual void Suggest(const FString& Word, TArray<FString>& OutSuggestions) = 0;
};

TSharedPtr<ISpellChecker> CreateSpellChecker();
