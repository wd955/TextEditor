#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Text/SlateEditableTextTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

class SScrollBar;

/**
 * Metadata describing a single token embedded in the text layout.
 * Stored as JSON within the run's metadata so that display names can
 * change without losing the original reference.
 */
struct FTokenMeta
{
        FString Table;
        FString Row;
        FString Column;

        FString ToDisplayString() const
        {
                return FString::Printf(TEXT("{{ %s.%s.%s }}"), *Table, *Row, *Column);
        }

        FString ToJson() const
        {
                TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
                Obj->SetStringField(TEXT("table"), Table);
                Obj->SetStringField(TEXT("row"), Row);
                Obj->SetStringField(TEXT("column"), Column);
                FString Out;
                TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
                FJsonSerializer::Serialize(Obj, Writer);
                return Out;
        }

        static FTokenMeta FromJson(const FString& In)
        {
                FTokenMeta Meta;
                TSharedPtr<FJsonObject> Obj;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(In);
                if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid())
                {
                        Obj->TryGetStringField(TEXT("table"), Meta.Table);
                        Obj->TryGetStringField(TEXT("row"), Meta.Row);
                        Obj->TryGetStringField(TEXT("column"), Meta.Column);
                }
                return Meta;
        }
};

class SRefTextEditor : public SCompoundWidget
{
public:
        SLATE_BEGIN_ARGS(SRefTextEditor)
                : _OnTextChanged()
                , _ExternalHScrollBar(nullptr)
                , _ExternalVScrollBar(nullptr)
        {}
                SLATE_EVENT(FOnTextChanged, OnTextChanged)
                SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalHScrollBar)
                SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalVScrollBar)
        SLATE_END_ARGS()

        void Construct(const FArguments& InArgs);
        virtual bool SupportsKeyboardFocus() const override { return true; }
        virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

        /** Set the delegate to notify on text changes after construction. */
        void SetOnTextChanged(FOnTextChanged InHandler);

        FString GetText() const;

        /** Serialize the text and token metadata to a JSON string. */
        FString Serialize() const;

        /** Restore the text and token metadata from a JSON string. */
        void Deserialize(const FString& InSerialized);

        /** Insert a token at the current cursor location. */
        void InsertToken(const FTokenMeta& Meta);

protected:
        virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
        // UI
        TSharedPtr<class SMultiLineEditableTextBox> TextBox;
        TSharedPtr<class STextBlock>               MisspellCounter;

        // Delegate to notify external widgets about text changes
        FOnTextChanged OnTextChanged;

        struct FMisspelling
        {
                FTextRange Range;
                FString    Word;
        };

        // cached misspelled ranges from the last scan
        TArray<FMisspelling> Misspellings;

        // Spell check is always enabled now
        void ScheduleSpellScan(); // simple immediate call
        void RunSpellScan();

        // context menu helpers
        TSharedPtr<SWidget> OnContextMenuOpening();
        void ReplaceWord(FMisspelling Miss, FString NewWord);

        void AddSelectionToDictionary();
        bool IsWordInCustomDictionary(const FString& Word) const;

        /** Build token suggestions from the master reference table. */
        TArray<FString> BuildTokenSuggestions() const;

        /** Show autocomplete menu populated with supplied suggestions. */
        void ShowSuggestions(const TArray<FString>& InSuggestions);

        /** Convert a token string (table.row.column) into metadata and insert. */
        void HandleSuggestionChosen(const FString& TokenString);

        // temporary storage for generated suggestions
        TArray<FString> CachedSuggestions;
};
