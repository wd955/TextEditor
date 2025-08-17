#include "Spell/SpellChecker.h"

class FDummySpellChecker : public ISpellChecker
{
public:
    virtual bool Check(const FString&) override { return true; }
    virtual void Suggest(const FString&, TArray<FString>&) override {}
};

TSharedPtr<ISpellChecker> CreateSpellChecker()
{
    return MakeShared<FDummySpellChecker>();
}
