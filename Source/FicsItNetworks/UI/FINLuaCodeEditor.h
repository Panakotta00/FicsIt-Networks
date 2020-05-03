#pragma once

#include "Slate.h"
#include "SyntaxHighlighterTextLayoutMarshaller.h"

#include "Widget.h"

#include "FINLuaCodeEditor.generated.h"

struct FFINLuaSyntaxTextStyle {
	FTextBlockStyle NormalTextStyle;
	FTextBlockStyle FunctionTextStyle;
	FTextBlockStyle StringTextStyle;
};

class FICSITNETWORKS_API FFINLuaSyntaxHighlighterTextLayoutMarshaller : public FSyntaxHighlighterTextLayoutMarshaller {
public:

	FFINLuaSyntaxHighlighterTextLayoutMarshaller(TSharedPtr<FSyntaxTokenizer> InTokenizer, FFINLuaSyntaxTextStyle InLuaSyntaxTextStyle);

	static TSharedRef<FFINLuaSyntaxHighlighterTextLayoutMarshaller> Create(FFINLuaSyntaxTextStyle LuaSyntaxTextStyle);

protected:
	virtual void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines) override;

	FFINLuaSyntaxTextStyle SyntaxTextStyle;
};

class SFINLuaCodeEditor : public SMultiLineEditableTextBox {
private:
	TSharedPtr<FFINLuaSyntaxHighlighterTextLayoutMarshaller> SyntaxHighlighter;

public:
	SLATE_BEGIN_ARGS(SFINLuaCodeEditor) {}
	SLATE_ARGUMENT(FTextBlockStyle, StyleNormal)
	SLATE_ARGUMENT(FTextBlockStyle, StyleFunction)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};


class SLuaEditor : public SCompoundWidget {
private:
	FSlateBrush BackgroundColor;

public:
	SLATE_BEGIN_ARGS(SLuaEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs) {

		BackgroundColor = FSlateColorBrush(FLinearColor::Black);

		ChildSlot.Padding(4)[
			SNew(SBorder).BorderImage(&BackgroundColor).BorderBackgroundColor(FSlateColor(FLinearColor::White))
				[

					SNew(SFINLuaCodeEditor)

				]
		];
	}
};

UCLASS()
class UFINLuaCodeEditor : public UWidget {
	GENERATED_BODY()
private:
	TSharedPtr<SFINLuaCodeEditor> CodeEditor;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FTextBlockStyle StyleNormal;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FTextBlockStyle StyleFunction;

	virtual TSharedRef<SWidget> RebuildWidget() override;
};