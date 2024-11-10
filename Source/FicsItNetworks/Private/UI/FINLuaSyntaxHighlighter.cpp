#include "FindLast.h"
#include "FINLuaSyntax.h"
#include "FINLuaCodeEditor.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
UE_DISABLE_OPTIMIZATION_SHIP
void FFINLuaSyntaxHighlighter::SetText(const FString& SourceString, FTextLayout& TargetTextLayout) {
	Lexer.Reset(TEXT(""));

	TArray<FTextRange> lineRanges;
	FTextRange::CalculateLineRangesFromString(SourceString, lineRanges);

	for (const FTextRange& lineRange : lineRanges) {
		TSharedRef<FString> str = MakeShared<FString>(SourceString.Mid(lineRange.BeginIndex, lineRange.Len()));
		Lexer.SetCode(*str);
		TArray<TSharedRef<IRun>> runs;

		bool bFunctionDeclaration = false;

		while (true) {
			FFINLuaLexer::EContext prevContext = Lexer.GetContext();
			auto optToken = Lexer.NextToken();
			if (!optToken.IsSet()) {
				if (runs.IsEmpty()) {
					FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
					FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Normal"));
					TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, FTextRange(0,0));
					runs.Add(Run.ToSharedRef());
				}
				break;
			}
			auto [token, textRange] = *optToken;
			FStringView text = FStringView(*str).Mid(textRange.BeginIndex, textRange.Len());
			if (token.IsSet() && Lexer.GetContext() == FFINLuaLexer::Context_Main && prevContext == FFINLuaLexer::Context_Main) {
				switch (*token) {
					case FIN_Lua_Token_Keyword:
					case FIN_Lua_Token_Nil:
					case FIN_Lua_Token_Operator: {
						FTextBlockStyle Style = SyntaxTextStyle->KeywordTextStyle;
						FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
						TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
						runs.Add(Run.ToSharedRef());
						if (text == TEXT("function")) {
							bFunctionDeclaration = true;
						}
						continue;
					}
					case FIN_Lua_Token_Boolean:
						if (text == TEXT("true")) {
							FTextBlockStyle Style = SyntaxTextStyle->BoolTrueTextStyle;
							FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
							TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
							runs.Add(Run.ToSharedRef());
						} else {
							FTextBlockStyle Style = SyntaxTextStyle->BoolFalseTextStyle;
							FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Keyword"));
							TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
							runs.Add(Run.ToSharedRef());
						}
						continue;
					case FIN_Lua_Token_Number: {
						FTextBlockStyle Style = SyntaxTextStyle->NumberTextStyle;
						FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Number"));
						TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
						runs.Add(Run.ToSharedRef());
						continue;
					}
					case FIN_Lua_Token_Parenthesis:
						if (text == TEXT("(")) {
							TSharedRef<IRun>* prevRun = Algo::FindLastByPredicate(runs, [](const TSharedRef<IRun>& run) {
								return run->GetRunInfo().Name == TEXT("SyntaxHighlight.FINLua.Identifier");
							});
							if (prevRun) {
								TSharedRef<IRun>& run = *prevRun;
								FTextBlockStyle style;
								if (bFunctionDeclaration) {
									style = SyntaxTextStyle->FunctionDeclarationTextStyle;
								} else {
									style = SyntaxTextStyle->FunctionCallTextStyle;
								}
								run = FSlateTextRun::Create(run->GetRunInfo(), str, style, run->GetTextRange());
							}
							bFunctionDeclaration = false;
						}
						break;
					case FIN_Lua_Token_Identifier: {
						int num = runs.Num();
						if (num >= 2) {
							int runBegin = runs[num-2]->GetTextRange().BeginIndex;
							auto prevText = FStringView(*str).Left(textRange.BeginIndex).Mid(runBegin);
							bool bClass = prevText == TEXT("classes.");
							bool bStruct = prevText == TEXT("structs.");
							if (bClass || bStruct) {
								FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.ReflectionReference"));
								FString Variant = bClass ? TEXT("class") : TEXT("struct");
								FString Type = FString(text);
								RunInfo.MetaData.Add(FFINReflectionReferenceDecorator::MetaDataVariantKey, Variant);
								RunInfo.MetaData.Add(FFINReflectionReferenceDecorator::MetaDataTypeKey, Type);
								bool bValid = nullptr != FFINReflectionReferenceDecorator::ReflectionItemFromType(Variant, Type);
								FHyperlinkStyle LinkStyle;
								LinkStyle.SetUnderlineStyle(bValid ? SyntaxTextStyle->UnderlineStyleValid : SyntaxTextStyle->UnderlineStyleInvalid);
								LinkStyle.SetTextStyle(SyntaxTextStyle->NormalTextStyle);
								runs.Pop();
								runs.Pop();
								runs.Add(FFINReflectionReferenceDecorator::CreateRun(RunInfo, str, &LinkStyle, FFINReflectionReferenceDecorator::FOnNavigate(), FTextRange(runBegin, textRange.EndIndex), true));
								continue;
							}
						}
						FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
						FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Identifier"));
						TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
						runs.Add(Run.ToSharedRef());
						continue;
					}
					case FIN_Lua_Token_Tab: {
						FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
						FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Normal"));
						runs.Add(FFINTabRun::Create(RunInfo, str, Style, textRange, 4));
						continue;
					}
					default: break;
				}
			}

			if (Lexer.GetContext() == FFINLuaLexer::Context_String || prevContext == FFINLuaLexer::Context_String) {
				FTextBlockStyle Style = SyntaxTextStyle->StringTextStyle;
				FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.String"));
				TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
				runs.Add(Run.ToSharedRef());
				continue;
			}

			if (Lexer.GetContext() == FFINLuaLexer::Context_Comment || prevContext == FFINLuaLexer::Context_Comment) {
				FTextBlockStyle Style = SyntaxTextStyle->CommentTextStyle;
				FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Comment"));
				TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
				runs.Add(Run.ToSharedRef());
				continue;
			}

			FTextBlockStyle Style = SyntaxTextStyle->NormalTextStyle;
			FRunInfo RunInfo(TEXT("SyntaxHighlight.FINLua.Normal"));
			TSharedPtr<IRun> Run = FSlateTextRun::Create(RunInfo, str, Style, textRange);
			runs.Add(Run.ToSharedRef());
		}
		TargetTextLayout.AddLine(FTextLayout::FNewLineData(str, runs));
	}
}
UE_ENABLE_OPTIMIZATION_SHIP