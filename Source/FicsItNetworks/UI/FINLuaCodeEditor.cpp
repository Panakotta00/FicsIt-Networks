#include "FINLuaCodeEditor.h"

const FName FFINLuaCodeEditorStyle::TypeName(TEXT("FFINLuaCodeEditorStyle"));

FFINLuaCodeEditorStyle::FFINLuaCodeEditorStyle() {}

void FFINLuaCodeEditorStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	Super::GetResources(OutBrushes);
}

const FFINLuaCodeEditorStyle& FFINLuaCodeEditorStyle::GetDefault() {
	static FFINLuaCodeEditorStyle* Default = nullptr;
	if (!Default) Default = new FFINLuaCodeEditorStyle();
	return *Default;
}

FFINLuaSyntaxHighlighterTextLayoutMarshaller::FFINLuaSyntaxHighlighterTextLayoutMarshaller(TSharedPtr<FSyntaxTokenizer> InTokenizer, const FFINLuaCodeEditorStyle* InLuaSyntaxTextStyle) :
	FSyntaxHighlighterTextLayoutMarshaller(InTokenizer),
	SyntaxTextStyle(InLuaSyntaxTextStyle) {

}

TSharedRef<FFINLuaSyntaxHighlighterTextLayoutMarshaller> FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(const FFINLuaCodeEditorStyle* LuaSyntaxTextStyle) {
	TArray<FSyntaxTokenizer::FRule> TokenizerRules;
	for (FString Token : TArray<FString>({" ", "\t", ".", ":", "\"", "\'", ",", "(", ")", "for", "in", "while", "do", "if", "then", "elseif", "else", "end", "local", "true", "false", "not", "and", "or", "function", "--[[", "]]--", "--", "+", "-", "/", "*", "%", "[", "]", "{", "}", "=", "!", "~"})) {
		TokenizerRules.Add(FSyntaxTokenizer::FRule(Token));
	}

	TokenizerRules.Sort([](const FSyntaxTokenizer::FRule& A, const FSyntaxTokenizer::FRule& B) {
		return A.MatchText.Len() > B.MatchText.Len();
	});
	
	return MakeShareable(new FFINLuaSyntaxHighlighterTextLayoutMarshaller(MakeShared<FSyntaxTokenizer>(TokenizerRules), LuaSyntaxTextStyle));
}
#pragma optimize("", off)
void FFINLuaSyntaxHighlighterTextLayoutMarshaller::ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines) {
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(TokenizedLines.Num());

	FString Input = "42,42";
	FString MatchText = ",";
	bool b1 = FCString::Strncmp(&Input[0], *MatchText, MatchText.Len()) == 0;
	bool b2 = FCString::Strncmp(&Input[1], *MatchText, MatchText.Len()) == 0;
	bool b3 = FCString::Strncmp(&Input[2], *MatchText, MatchText.Len()) == 0;
	bool b4 = FCString::Strncmp(&Input[3], *MatchText, MatchText.Len()) == 0;
	bool b5 = FCString::Strncmp(&Input[4], *MatchText, MatchText.Len()) == 0;
	bool bInString = false;
	bool bInBlockComment = false;
	TSharedPtr<ISlateRun> Run;
	for (const FSyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines) {
		TSharedRef<FString> ModelString = MakeShareable(new FString());
		TArray<TSharedRef<IRun>> Runs;

		auto DoNormal = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Normal"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoComment = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->CommentTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Comment"));
			RunInfo.MetaData.Add("Splitting");
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoString = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->StringTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.String"));
			RunInfo.MetaData.Add("Splitting");
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoKeyword = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->KeywordTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoTrue = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->BoolTrueTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoFalse = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->BoolFalseTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoNumber = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->NumberTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Number"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoOperator = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->OperatorTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Operator"));
			RunInfo.MetaData.Add("Splitting");
			RunInfo.MetaData.Add("Operator", ModelString->Mid(Range.BeginIndex, Range.Len()));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};

		int StringStart = 0;
		int StringEnd = 0;
		bool bInNumber = false;
		bool bNumberHadDecimal = false;
		bool bInLineComment = false;
		for (const FSyntaxTokenizer::FToken& Token : TokenizedLine.Tokens) {
			const FString TokenString = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());
			int Start = ModelString->Len();
			int End = Start + TokenString.Len();
			ModelString->Append(TokenString);

			bool bIsNew = !Run.IsValid() || Start < 1 || Run->GetRunInfo().MetaData.Contains("Splitting");
			
			if (bInString || bInLineComment || bInBlockComment) {
				StringEnd += TokenString.Len();
			}
			if (!bInBlockComment && !bInLineComment && (TokenString == "\"" || TokenString == "\'")) {
				if (bInString) {
					if (Start > 0 && (*ModelString)[Start-1] == '\\') continue;
					DoString(FTextRange(StringStart, StringEnd));
					bInString = false;
					continue;
				}
				bInString = true;
				StringStart = Start;
				StringEnd = End;
				continue;
			}
			if (!bInString) {
				if (TokenString == "--[[" && !bInBlockComment && !bInLineComment) {
					if (!bInBlockComment) {
						bInBlockComment = true;
						StringStart = Start;
						StringEnd = End;
					}
				} else if (TokenString == "]]--" && bInBlockComment) {
					bInBlockComment = false;
					DoComment(FTextRange(StringStart, StringEnd));
					continue;
				} else if (TokenString == "--" && !bInLineComment && !bInBlockComment) {
					bInLineComment = true;
					StringStart = Start;
					StringEnd = End;
				}
			}
			if (bInString || bInLineComment || bInBlockComment) continue;
			if (bInNumber) {
				bool bStillNumber = false;
				if (TokenString == ".") {
					if (!bNumberHadDecimal) {
						bNumberHadDecimal = true;
						bStillNumber = true;
					}
				} else if (FRegexMatcher(FRegexPattern("^[0-9]+$"), TokenString).FindNext()) bStillNumber = true;
				if (bStillNumber) {
					StringEnd += TokenString.Len();
					continue;
				}
				DoNumber(FTextRange(StringStart, StringEnd));
				bIsNew = true;
				bInNumber = false;
			}

			if (Token.Type == FSyntaxTokenizer::ETokenType::Syntax) {
				if (bIsNew) {
					if (TArray<FString>({"while", "for", "in", "do", "if", "then", "elseif", "else", "end", "local", "not", "and", "or", "function"}).Contains(TokenString)) {
						DoKeyword(FTextRange(Start, End));
						continue;
					} else if (TokenString == "true") {
						DoTrue(FTextRange(Start, End));
						continue;
					} else if (TokenString == "false") {
						DoFalse(FTextRange(Start, End));
						continue;
					}
				}
				if (TArray<FString>({",",":","+","-","*","/","%","(",")","[","]","{","}","=","~","!"," ","\t"}).Contains(TokenString)) {
					DoOperator(FTextRange(Start, End));
					continue;
				}
			} else {
				if (!bIsNew) {
					FTextRange ModelRange = Run->GetTextRange();
					Runs.RemoveAt(Runs.Num()-1);
					DoNormal(ModelRange);
					bIsNew = false;
				} else if (TokenString.IsNumeric()) {
					bInNumber = true;
					StringStart = Start;
					StringEnd = End;
					continue;
				}
			}
			DoNormal(FTextRange(Start, End));
		}
		
		if (bInNumber) {
			DoNumber(FTextRange(StringStart, StringEnd));
		} else if (bInString) {
			DoString(FTextRange(StringStart, StringEnd));
		} else if (bInLineComment || bInBlockComment) {
			DoComment(FTextRange(StringStart, StringEnd));
		}
		
		LinesToAdd.Emplace(MoveTemp(ModelString), MoveTemp(Runs));
	}
	TargetTextLayout.AddLines(LinesToAdd);
}
#pragma optimize("", on)

void SFINLuaCodeEditor::Construct(const FArguments& InArgs) {
	SyntaxHighlighter = FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(InArgs._CodeStyle);

	ChildSlot[
		SAssignNew(TextBox, SMultiLineEditableTextBox)
		.AutoWrapText(false)
		.Margin(0.0f)
		.Padding(InArgs._Padding)
		.Marshaller(SyntaxHighlighter)
		.Style(InArgs._Style)
		/*.OnIsTypedCharValid_Lambda([](TCHAR InChar) {
			return true;
		})*/
	];
}

TSharedRef<SWidget> UFINLuaCodeEditor::RebuildWidget() {
	CodeEditor =
		SNew(SFINLuaCodeEditor)
		.Style(&Style)
		.CodeStyle(&CodeStyle);
	return CodeEditor.ToSharedRef();
}

void UFINLuaCodeEditor::SetIsReadOnly(bool bInReadOnly) {
	CodeEditor->TextBox->SetIsReadOnly(bInReadOnly);
}

void UFINLuaCodeEditor::SetText(FText InText) {
	CodeEditor->TextBox->SetText(InText);
}

FText UFINLuaCodeEditor::GetText() const {
	return CodeEditor->TextBox->GetText();
}

