#include "FINLuaCodeEditor.h"

FFINLuaSyntaxHighlighterTextLayoutMarshaller::FFINLuaSyntaxHighlighterTextLayoutMarshaller(TSharedPtr<FSyntaxTokenizer> InTokenizer, FFINLuaSyntaxTextStyle InLuaSyntaxTextStyle) :
	FSyntaxHighlighterTextLayoutMarshaller(InTokenizer),
	SyntaxTextStyle(InLuaSyntaxTextStyle) {

}

TSharedRef<FFINLuaSyntaxHighlighterTextLayoutMarshaller> FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(FFINLuaSyntaxTextStyle LuaSyntaxTextStyle) {
	TArray<FSyntaxTokenizer::FRule> TokenizerRules;
	TokenizerRules.Add(FSyntaxTokenizer::FRule(TEXT("print")));
	TokenizerRules.Add(FSyntaxTokenizer::FRule(TEXT("\"")));

	TokenizerRules.Sort([](const FSyntaxTokenizer::FRule& A, const FSyntaxTokenizer::FRule& B) {
		return A.MatchText.Len() > B.MatchText.Len();
	});


	return MakeShareable(new FFINLuaSyntaxHighlighterTextLayoutMarshaller(FSyntaxTokenizer::Create(TokenizerRules), LuaSyntaxTextStyle));
}

void FFINLuaSyntaxHighlighterTextLayoutMarshaller::ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines) {
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(TokenizedLines.Num());

	for (const FSyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines) {
		TSharedRef<FString> ModelString = MakeShareable(new FString());
		TArray<TSharedRef<IRun>> Runs;

		for (const FSyntaxTokenizer::FToken& Token : TokenizedLine.Tokens) {
			const FString TokenString = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());
			const FTextRange ModelRange(ModelString->Len(), ModelString->Len() + TokenString.Len());

			ModelString->Append(TokenString);

			FTextBlockStyle CurrentBlockStyle = SyntaxTextStyle.NormalTextStyle;

			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Normal"));

			if (TokenString == "print") {
				RunInfo.Name = TEXT("SyntaxHighlight.FINLua.Function");
				CurrentBlockStyle = SyntaxTextStyle.FunctionTextStyle;
			}

			TSharedRef<ISlateRun> Run = FSlateTextRun::Create(RunInfo, ModelString, CurrentBlockStyle, ModelRange);
			Runs.Add(Run);
		}
		LinesToAdd.Emplace(MoveTemp(ModelString), MoveTemp(Runs));
	}
	TargetTextLayout.AddLines(LinesToAdd);
}


void SFINLuaCodeEditor::Construct(const FArguments& InArgs) {
	FFINLuaSyntaxTextStyle LuaTextStyle;

	LuaTextStyle.NormalTextStyle = InArgs._StyleNormal;
	LuaTextStyle.FunctionTextStyle = InArgs._StyleFunction;
	
	SyntaxHighlighter = FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(LuaTextStyle);

	SMultiLineEditableTextBox::Construct(
		SMultiLineEditableTextBox::FArguments()
		.AutoWrapText(false)
		.Margin(0.0f)
		.Text(FText::FromString(TEXT("print(\"test\")")))
		.Marshaller(SyntaxHighlighter)
		.BackgroundColor(FSlateColor(FLinearColor::Black))
		.ForegroundColor(FSlateColor(FLinearColor::White))
	);
}

TSharedRef<SWidget> UFINLuaCodeEditor::RebuildWidget() {
	FTextBlockStyle styleN;
	
	CodeEditor =
		SNew(SFINLuaCodeEditor)
		.StyleNormal(StyleNormal)
		.StyleFunction(StyleFunction);
	return CodeEditor.ToSharedRef();
}
