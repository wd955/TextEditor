#include "Spell/SpellChecker.h"

#if !REFTEXT_WINDOWS_SPELL

class FDummySpellChecker : public IRefSpellChecker
{
public:
    virtual bool Check(const FString&) override { return true; }
    virtual void Suggest(const FString&, TArray<FString>&) override {}
};

TSharedPtr<IRefSpellChecker> CreateSpellChecker()
{
    return MakeShared<FDummySpellChecker>();
}

#endif // !REFTEXT_WINDOWS_SPELL
