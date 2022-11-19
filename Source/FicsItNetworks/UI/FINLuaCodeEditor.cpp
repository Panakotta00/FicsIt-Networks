#include "FINLuaCodeEditor.h"

#include "FicsItNetworks/FicsItNetworksModule.h"

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

void FFINLuaSyntaxHighlighterTextLayoutMarshaller::SetText(const FString& SourceString, FTextLayout& TargetTextLayout) {
	TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines;
	TArray<FTextRange> LineRanges;
	FTextRange::CalculateLineRangesFromString(SourceString, LineRanges);

	TArray<FString> Rules = TArray<FString>({
		" ", "\t", "\\.", "\\:", "\\\"", "\\\'", "\\,", "\\\\", "\\(", "\\)", "for", "in", "while", "do", "if", "then", "elseif", "else",
		"end", "local", "true", "false", "not", "and", "or", "function", "return", "--\\[\\[", "\\]\\]--", "--", "\\+", "\\-", "\\/",
		"\\*", "\\%", "\\[", "\\]", "\\{", "\\}", "\\=", "\\!", "\\~", "\\#", "\\>", "\\<"});
	
	FString Pat;
	for (const FString& Rule : Rules) {
		Pat += FString::Printf(TEXT("(%s)|"), *Rule);
	}
	if (Rules.Num() > 0) Pat = Pat.LeftChop(1);
	FRegexPattern Pattern(Pat);

	for (const FTextRange& LineRange : LineRanges) {
		FSyntaxTokenizer::FTokenizedLine TokenizedLine;
		TokenizedLine.Range = LineRange;

		FString Line = SourceString.Mid(LineRange.BeginIndex, LineRange.EndIndex - LineRange.BeginIndex);
		FRegexMatcher Match(Pattern, Line);
		int32 Start = 0;
		int32 End = 0;
		while (Match.FindNext()) {
			int32 MatchStart = Match.GetMatchBeginning();
			End = Match.GetMatchEnding();
			if (MatchStart != Start) {
				TokenizedLine.Tokens.Add(FSyntaxTokenizer::FToken(FSyntaxTokenizer::ETokenType::Literal, FTextRange(LineRange.BeginIndex + Start, LineRange.BeginIndex + MatchStart)));
			}
			Start = End;
			TokenizedLine.Tokens.Add(FSyntaxTokenizer::FToken(FSyntaxTokenizer::ETokenType::Syntax, FTextRange(LineRange.BeginIndex + MatchStart, LineRange.BeginIndex + End)));
		}
		if (End < LineRange.EndIndex - LineRange.BeginIndex || TokenizedLine.Tokens.Num() < 1) {
			TokenizedLine.Tokens.Add(FSyntaxTokenizer::FToken(FSyntaxTokenizer::ETokenType::Syntax, FTextRange(LineRange.BeginIndex + End, LineRange.EndIndex)));
		}
		TokenizedLines.Add(TokenizedLine);
	}
	
	ParseTokens(SourceString, TargetTextLayout, TokenizedLines);
}

bool FFINLuaSyntaxHighlighterTextLayoutMarshaller::RequiresLiveUpdate() const {
	return true;
}

FFINLuaSyntaxHighlighterTextLayoutMarshaller::FFINLuaSyntaxHighlighterTextLayoutMarshaller(const FFINLuaCodeEditorStyle* InLuaSyntaxTextStyle) : SyntaxTextStyle(InLuaSyntaxTextStyle) {

}

TSharedRef<FFINLuaSyntaxHighlighterTextLayoutMarshaller> FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(const FFINLuaCodeEditorStyle* LuaSyntaxTextStyle) {
	TArray<FSyntaxTokenizer::FRule> TokenizerRules;
	for (FString Token : TArray<FString>({
		" ", "\t", ".", ":", "\"", "\'", "\\", ",", "(", ")", "for", "in", "while", "do", "if", "then", "elseif", "else",
		"end", "local", "true", "false", "not", "and", "or", "function", "return", "--[[", "]]--", "--", "+", "-", "/",
		"*", "%", "[", "]", "{", "}", "=", "!", "~", "#", ">", "<"})) {
		TokenizerRules.Add(FSyntaxTokenizer::FRule(Token));
	}

	TokenizerRules.Sort([](const FSyntaxTokenizer::FRule& A, const FSyntaxTokenizer::FRule& B) {
		return A.MatchText.Len() > B.MatchText.Len();
	});
	
	return MakeShareable(new FFINLuaSyntaxHighlighterTextLayoutMarshaller(LuaSyntaxTextStyle));
}

void FFINLuaSyntaxHighlighterTextLayoutMarshaller::ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines) {
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(TokenizedLines.Num());

	bool bInString = false;
	bool bInBlockComment = false;
	TSharedPtr<ISlateRun> Run;
	for (const FSyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines) {
		TSharedRef<FString> ModelString = MakeShareable(new FString());
		TArray<TSharedRef<IRun>> Runs;

		auto DoNormal = [&](FTextRange Range) {
			if (Runs.Num() > 0 && Runs[Runs.Num()-1]->GetRunInfo().Name == "SyntaxHighlight.FINLua.Normal") {
				Range.BeginIndex = Runs[Runs.Num()-1]->GetTextRange().BeginIndex;
				Runs.Pop();
			}
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
		auto DoWhitespace = [&](const FTextRange& Range) {
			FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Whitespace"));
			RunInfo.MetaData.Add("Splitting");
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoOperator = [&](const FTextRange& Range, bool bColored) {
			FTextBlockStyle Style = bColored ? SyntaxTextStyle->OperatorTextStyle : SyntaxTextStyle->NormalTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Operator"));
			RunInfo.MetaData.Add("Splitting");
			RunInfo.MetaData.Add("Operator", ModelString->Mid(Range.BeginIndex, Range.Len()));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto DoFunction = [&](const FTextRange& Range, bool bDeclaration) {
			FTextBlockStyle Style = bDeclaration ? SyntaxTextStyle->FunctionDeclarationTextStyle : SyntaxTextStyle->FunctionCallTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Function"));
			Run = FSlateTextRun::Create(RunInfo, ModelString, Style, Range);
			Runs.Add(Run.ToSharedRef());
		};
		auto FindPrevNoWhitespaceRun = [&](int32 StartIndex = -1) {
			if (StartIndex < 0) StartIndex = Runs.Num()-1;
			for (int i = StartIndex; i >= 0; i--) {
				if (Runs[i]->GetRunInfo().Name != "SyntaxHighlight.FINLua.Whitespace") {
					return i;
				}
			}
			return -1;
		};

		int StringStart = 0;
		int StringEnd = 0;
		bool bInNumber = false;
		bool bNumberHadDecimal = false;
		bool bInLineComment = false;
		bool bIsEscaped = false;
		for (const FSyntaxTokenizer::FToken& Token : TokenizedLine.Tokens) {
			const FString TokenString = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());
			int Start = ModelString->Len();
			int End = Start + TokenString.Len();
			ModelString->Append(TokenString);
			bool bWasEscaped = bIsEscaped;
			bIsEscaped = false;

			bool bIsNew = !Run.IsValid() || Start < 1 || Run->GetRunInfo().MetaData.Contains("Splitting");
			
			if (bInString || bInLineComment || bInBlockComment) {
				StringEnd += TokenString.Len();
			}
			if (!bInBlockComment && !bInLineComment && (TokenString == "\\")) {
				if (bInString) {
					bIsEscaped = !bWasEscaped;
					continue;
				}
			}
			if (!bInBlockComment && !bInLineComment && (TokenString == "\"" || TokenString == "\'")) {
				if (bInNumber) {
					DoNumber(FTextRange(StringStart, StringEnd));
					bIsNew = true;
					bInNumber = false;
				}
				if (bInString) {
					if (Start > 0 && bWasEscaped) continue;
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
					if (bInNumber) {
						bInNumber = false;
						DoNumber(FTextRange(StringStart, StringEnd));
					}
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
					if (bInNumber) {
						bInNumber = false;
						DoNumber(FTextRange(StringStart, StringEnd));
					}
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
					if (TArray<FString>({"while", "for", "in", "do", "if", "then", "elseif", "else", "end", "local", "not", "and", "or", "function", "return"}).Contains(TokenString)) {
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
				if (TokenString == "(") {
					int Index = FindPrevNoWhitespaceRun();
					if (Index >= 0 && Runs[Index]->GetRunInfo().Name == "SyntaxHighlight.FINLua.Normal") {
						FTextRange OldRange = Runs[Index]->GetTextRange();
						Runs.RemoveAt(Index);
						int KeywordIndex = FindPrevNoWhitespaceRun(Index-1);
						FString Keyword;
						if (KeywordIndex >= 0) Keyword = ModelString->Mid(Runs[KeywordIndex]->GetTextRange().BeginIndex, Runs[KeywordIndex]->GetTextRange().EndIndex - Runs[KeywordIndex]->GetTextRange().BeginIndex);
                        DoFunction(OldRange, KeywordIndex >= 0 && Runs[KeywordIndex]->GetRunInfo().Name == "SyntaxHighlight.FINLua.Keyword" && Keyword == "function");
						TSharedRef<IRun> NewRun = Runs[Runs.Num()-1];
						Runs.RemoveAt(Runs.Num()-1);
						Runs.Insert(NewRun, Index);
						DoOperator(FTextRange(Start, End), false);
						continue;
					}
				}
				if (TArray<FString>({" ","\t"}).Contains(TokenString)) {
					DoWhitespace(FTextRange(Start, End));
					continue;
				}
				if (TArray<FString>({".",",",":","(",")","[","]","{","}"}).Contains(TokenString)) {
					DoOperator(FTextRange(Start, End), false);
					continue;
				}
				if (TArray<FString>({"+","-","*","/","%","#","=","~","!",">","<"}).Contains(TokenString)) {
					DoOperator(FTextRange(Start, End), true);
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
			if (TokenString.IsNumeric()) DoNumber(FTextRange(Start, End));
			else DoNormal(FTextRange(Start, End));
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

void SFINLuaCodeEditor::Construct(const FArguments& InArgs) {
	SyntaxHighlighter = FFINLuaSyntaxHighlighterTextLayoutMarshaller::Create(InArgs._CodeStyle);

	ChildSlot[
		SAssignNew(TextBox, SMultiLineEditableTextBox)
		.AutoWrapText(false)
		.Margin(0.0f)
		.Padding(InArgs._Padding)
		.Marshaller(SyntaxHighlighter)
		.Style(InArgs._Style)
		.OnTextChanged(InArgs._OnTextChanged)
		.OnTextCommitted(InArgs._OnTextCommitted)
		.TextShapingMethod(ETextShapingMethod::Auto)
	];
}

void UFINLuaCodeEditor::HandleOnTextChanged(const FText& Text) {
	OnTextChanged.Broadcast(Text);
}

void UFINLuaCodeEditor::HandleOnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod) {
	OnTextCommitted.Broadcast(Text, CommitMethod);
}

TSharedRef<SWidget> UFINLuaCodeEditor::RebuildWidget() {
	return SAssignNew(CodeEditor, SFINLuaCodeEditor)
		.Style(&Style)
		.CodeStyle(&CodeStyle)
		.OnTextChanged(BIND_UOBJECT_DELEGATE(FOnTextChanged, HandleOnTextChanged))
		.OnTextCommitted(BIND_UOBJECT_DELEGATE(FOnTextCommitted, HandleOnTextCommitted));
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

