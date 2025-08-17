#if REFTEXT_WINDOWS_SPELL

#include "Spell/SpellChecker.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <wrl/client.h>
#include <combaseapi.h>
#include <spellcheck.h>
#include "Windows/HideWindowsPlatformTypes.h"

using Microsoft::WRL::ComPtr;

static BSTR MakeBSTR(const FString& In)
{
    return ::SysAllocStringLen((const OLECHAR*)*In, In.Len());
}

class FWinSpellChecker : public IRefSpellChecker
{
public:
    FWinSpellChecker()
    {
        // Initialize COM STA once for this module
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        ComPtr<ISpellCheckerFactory> Factory;
        HRESULT hr = CoCreateInstance(CLSID_SpellCheckerFactory, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&Factory));
        if (SUCCEEDED(hr) && Factory)
        {
            // Try user default first; if that fails, fall back to en-US
            hr = Factory->CreateSpellChecker(nullptr, &Checker);
            if (FAILED(hr) || !Checker)
            {
                Factory->CreateSpellChecker(L"en-US", &Checker);
            }
        }
    }

    virtual ~FWinSpellChecker() override
    {
        // Let COM uninitialize when module unloads; not strictly necessary here.
    }

    virtual bool Check(const FString& Word) override
    {
        if (!Checker) return true;

        ComPtr<IEnumSpellingError> Errors;
        TScopedBSTR TextBSTR(MakeBSTR(Word));
        if (!TextBSTR) return true;

        if (SUCCEEDED(Checker->Check(TextBSTR, &Errors)) && Errors)
        {
            ComPtr<ISpellingError> Err;
            return !(Errors->Next(&Err) == S_OK); // true if NO error
        }
        return true; // assume ok if API failed
    }

    virtual void Suggest(const FString& Word, TArray<FString>& OutSuggestions) override
    {
        OutSuggestions.Reset();
        if (!Checker) return;

        ComPtr<IEnumString> Suggestions;
        TScopedBSTR TextBSTR(MakeBSTR(Word));
        if (!TextBSTR) return;

        if (SUCCEEDED(Checker->Suggest(TextBSTR, &Suggestions)) && Suggestions)
        {
            LPOLESTR Item = nullptr;
            ULONG Fetched = 0;
            while (Suggestions->Next(1, &Item, &Fetched) == S_OK && Item)
            {
                OutSuggestions.Add(FString(Item));
                CoTaskMemFree(Item);
                Item = nullptr;
            }
        }
    }

private:
    struct TScopedBSTR
    {
        BSTR Ptr{nullptr};
        explicit TScopedBSTR(BSTR In) : Ptr(In) {}
        ~TScopedBSTR(){ if(Ptr) ::SysFreeString(Ptr); }
        operator BSTR() const { return Ptr; }
        explicit operator bool() const { return Ptr != nullptr; }
    };

    ComPtr<::ISpellChecker> Checker;
};

TSharedPtr<IRefSpellChecker> CreateSpellChecker()
{
    return MakeShared<FWinSpellChecker>();
}

#endif // REFTEXT_WINDOWS_SPELL
