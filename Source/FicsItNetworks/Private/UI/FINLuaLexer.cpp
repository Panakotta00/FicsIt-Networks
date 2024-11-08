#include "UI/FINLuaSyntax.h"
#include "Internationalization/Regex.h"

TArray<FString> FIN_Lua_Keywords = {

};

TArray<FFINLexerPattern> FIN_Lua_MainTokens = {
	{R"(\[(=*)\[)", FIN_Lua_Token_String_Long, 1},
	{R"(--\[(=*)\[)", FIN_Lua_Token_Comment_Long, 1},
	{"--", FIN_Lua_Token_Comment},

	{"and(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"or(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"not(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"function(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"if(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"else(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"elseif(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"then(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"for(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"while(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"in(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"repeat(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"until(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"do(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"break(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"end(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"goto(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"local(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},
	{"return(?![a-zA-Z0-9])", FIN_Lua_Token_Keyword},

	{"::([a-zA-Z_][a-zA-Z0-9_]*)::", FIN_Lua_Token_Label, 1},

	{"true(?![a-zA-Z0-9])", FIN_Lua_Token_Boolean},
	{"false(?![a-zA-Z0-9])", FIN_Lua_Token_Boolean},
	{"nil(?![a-zA-Z0-9])", FIN_Lua_Token_Nil},
	{R"(0[xX][0-9a-fA-F]*\.[0-9a-fA-F]+([pP][+-]?[0-9]+)?)", FIN_Lua_Token_Number, 1},
	{R"(0[xX][0-9a-fA-F]+\.?[0-9a-fA-F]*([pP][+-]?[0-9]+)?)", FIN_Lua_Token_Number, 1},
	{R"([0-9]*\.[0-9]+([eE][+-]?[0-9]+)?)", FIN_Lua_Token_Number, 1},
	{R"([0-9]+\.?[0-9]*([eE][+-]?[0-9]+)?)", FIN_Lua_Token_Number, 1},

	{R"(\+)", FIN_Lua_Token_Operator},
	{R"(\-)", FIN_Lua_Token_Operator},
	{R"(\*)", FIN_Lua_Token_Operator},
	{R"(\/)", FIN_Lua_Token_Operator},
	{R"(\%)", FIN_Lua_Token_Operator},
	{R"(\^)", FIN_Lua_Token_Operator},
	{R"(//)", FIN_Lua_Token_Operator},

	{R"(\&)", FIN_Lua_Token_Operator},
	{R"(\~)", FIN_Lua_Token_Operator},
	{R"(\|)", FIN_Lua_Token_Operator},
	{R"(<<)", FIN_Lua_Token_Operator},
	{R"(>>)", FIN_Lua_Token_Operator},

	{R"(\==)", FIN_Lua_Token_Operator},
	{R"(\~=)", FIN_Lua_Token_Operator},
	{R"(\<=)", FIN_Lua_Token_Operator},
	{R"(\>=)", FIN_Lua_Token_Operator},
	{R"(\<)", FIN_Lua_Token_Operator},
	{R"(\>)", FIN_Lua_Token_Operator},
	{R"(\=)", FIN_Lua_Token_Operator},

	{R"(\#)", FIN_Lua_Token_Operator},
	{R"(\.\.)", FIN_Lua_Token_Operator},
	{R"(\.\.\.)", FIN_Lua_Token_Operator},

	{"'", FIN_Lua_Token_Quote},
	{"\"", FIN_Lua_Token_Quote},

	{R"(\.)", FIN_Lua_Token_Access},
	{":", FIN_Lua_Token_Access},

	{",", FIN_Lua_Token_Comma},

	{";", FIN_Lua_Token_Separator},

	{R"(\()", FIN_Lua_Token_Parenthesis},
	{R"(\))", FIN_Lua_Token_Parenthesis},
	{R"(\[)", FIN_Lua_Token_ParenthesisSquare},
	{R"(\])", FIN_Lua_Token_ParenthesisSquare},
	{R"(\{)", FIN_Lua_Token_ParenthesisCurly},
	{R"(\})", FIN_Lua_Token_ParenthesisCurly},

	{"[ \\t]+", FIN_Lua_Token_WhiteSpace},
	{"[a-zA-Z_][a-zA-Z0-9_]*", FIN_Lua_Token_Identifier},
};

TArray<FFINLexerPattern> FIN_Lua_StringTokens = {
	{R"(\](=*)\])", FIN_Lua_Token_LongBracketClose, 1},

	{R"(\\a)", FIN_Lua_Token_String_Escape},
	{R"(\\b)", FIN_Lua_Token_String_Escape},
	{R"(\\f)", FIN_Lua_Token_String_Escape},
	{R"(\\n)", FIN_Lua_Token_String_Escape},
	{R"(\\r)", FIN_Lua_Token_String_Escape},
	{R"(\\t)", FIN_Lua_Token_String_Escape},
	{R"(\\v)", FIN_Lua_Token_String_Escape},
	{R"(\\\\)", FIN_Lua_Token_String_Escape},
	{R"(\\")", FIN_Lua_Token_String_Escape},
	{R"(\\')", FIN_Lua_Token_String_Escape},
	{R"(\\z)", FIN_Lua_Token_String_Escape},
	{R"(\\x[0-9a-fA-F][0-9a-fA-F])", FIN_Lua_Token_String_Escape},
	{R"(\\[0-9][0-9]?[0-9]?)", FIN_Lua_Token_String_Escape},
	{R"(\\\{[0-9a-fA-F]+\})", FIN_Lua_Token_String_Escape},

	{"'", FIN_Lua_Token_Quote},
	{"\"", FIN_Lua_Token_Quote},
};

TArray<FFINLexerPattern> FIN_Lua_CommentTokens = {
	{R"(\](=*)\])", FIN_Lua_Token_LongBracketClose, 1},
};

FFINLexerRuleSet FIN_Lua_CreateLexerPattern(TArrayView<FFINLexerPattern> Tokens) {
	TMap<EFINLuaToken, int> groups;
	TMap<int, EFINLuaToken> tokenMap;
	FStringBuilderBase str;

	int group = 0;
	for (const auto& token : Tokens) {
		str.Appendf(TEXT("(%s)|"), *token.Pattern);
		groups.Add(token.Token, ++group);
		tokenMap.Add(group, token.Token);
		group += token.Groups;
	}

	str.RemoveSuffix(1);

	FRegexPattern pattern(str.ToString());

	return {pattern, groups, tokenMap};
}

TOptional<TTuple<EFINLuaToken, FTextRange>> FFINLexerRuleSet::FindMatchedToken(FRegexMatcher& Matcher, const FStringView Code) const {
	for (const auto& [group, token] : TokenMap) {
		int32 beginning = Matcher.GetCaptureGroupBeginning(group);
		if (beginning == INDEX_NONE) {
			continue;
		}
		int32 ending = Matcher.GetCaptureGroupEnding(group);

		FTextRange matched = FTextRange(beginning, ending);
		FStringView text = Code.Mid(matched.BeginIndex, matched.Len());

		return {{token, matched}};
	}

	return {};
}

bool FFINLuaLexer::bInitialized = false;
FFINLexerRuleSet FFINLuaLexer::MainContext;
FFINLexerRuleSet FFINLuaLexer::StringContext;
FFINLexerRuleSet FFINLuaLexer::CommentContext;

void FFINLuaLexer::Initialize() {
	if (bInitialized) return;

	MainContext = FIN_Lua_CreateLexerPattern(FIN_Lua_MainTokens);
	StringContext = FIN_Lua_CreateLexerPattern(FIN_Lua_StringTokens);
	CommentContext = FIN_Lua_CreateLexerPattern(FIN_Lua_CommentTokens);
}

const FFINLexerRuleSet& FFINLuaLexer::GetContext(EContext context) {
	switch (context) {
		case Context_Main:
			return MainContext;
		case Context_String:
			return StringContext;
		case Context_Comment:
			return CommentContext;
	}
	return MainContext;
}

void FFINLuaLexer::SetContext(EContext Context) {
	CurrentContext = Context;
	if (CurrentMatcher) {
		const FFINLexerRuleSet& context = GetContext(CurrentContext);
		//int32 beginLimit = CurrentMatcher->GetBeginLimit();
		//int32 endLimit = CurrentMatcher->GetEndLimit();
		offset += prevEnd;
		prevEnd = 0;
		CurrentMatcher = FRegexMatcher(context.Regex, FString(Code.Mid(offset)));
		//CurrentMatcher->SetLimits(beginLimit, endLimit);
	} else {
		SetCode(Code);
	}
}

void FFINLuaLexer::Reset(const FStringView InCode) {
	BlockLevel.Reset();
	Quote.Reset();
	Code = InCode;
	CurrentMatcher = {};
	SetContext(Context_Main);
}

void FFINLuaLexer::SetCode(const FStringView InCode) {
	Code = InCode;
	prevEnd = 0;
	offset = 0;
	bFound = false;
	const FFINLexerRuleSet& context = GetContext(CurrentContext);
	CurrentMatcher = FRegexMatcher(context.Regex, FString(Code));
}
UE_DISABLE_OPTIMIZATION_SHIP
TOptional<FFINLuaToken> FFINLuaLexer::NextToken() {
	const FFINLexerRuleSet& context = GetContext(CurrentContext);
	FRegexMatcher& matcher = *CurrentMatcher;

	if (offset+prevEnd >= Code.Len()) {
		// Reached end of string -> Return no Token
		if (!BlockLevel.IsSet()) {
			SetContext(Context_Main);
		}
		return {};
	}

	if (!bFound) {
		if (!matcher.FindNext()) {
			// No match found -> Return Invalid Token containing rest of string
			FTextRange range(prevEnd+offset, Code.Len());
			prevEnd = Code.Len()-offset;
			return FFINLuaToken(range);
		}

		int begin = matcher.GetMatchBeginning();
		if (prevEnd != begin) {
			// Match found but it skipped some characters -> Mark next call to use match & now return Invalid Token Containing skipped characters
			bFound = true;
			FTextRange range(prevEnd+offset, matcher.GetMatchBeginning()+offset);
			prevEnd = matcher.GetMatchEnding()-1;
			return FFINLuaToken(range);
		}
	}
	bFound = false;

	prevEnd = matcher.GetMatchEnding();

	const TOptional<TTuple<EFINLuaToken, FTextRange>> optionalToken = context.FindMatchedToken(matcher, Code.Mid(offset));
	if (!optionalToken.IsSet()) {
		// Matched but no Token Enum found -> Return Invalid Token with matched string
		FTextRange range(matcher.GetMatchBeginning()+offset, matcher.GetMatchEnding()+offset);
		return FFINLuaToken(range);
	}
	EFINLuaToken token = optionalToken->Get<0>();
	FTextRange textRange = optionalToken->Get<1>();
	textRange.BeginIndex += offset;
	textRange.EndIndex += offset;
	FStringView text = Code.Left(textRange.EndIndex).Mid(textRange.BeginIndex);

	switch (token) {
		case FIN_Lua_Token_Quote:
			if (Quote) {
				if (Quote == text[0]) {
					SetContext(Context_Main);
					Quote.Reset();
				}
			} else {
				Quote = text[0];
				SetContext(Context_String);
			}
			break;
		case FIN_Lua_Token_Comment_Long: {
			int group = context.GroupMap[FIN_Lua_Token_Comment_Long]+1;
			int level = matcher.GetCaptureGroupEnding(group) - matcher.GetCaptureGroupBeginning(group);
			BlockLevel = level;
		} case FIN_Lua_Token_Comment:
			SetContext(Context_Comment);
			break;
		case FIN_Lua_Token_String_Long: {
			int group = context.GroupMap[FIN_Lua_Token_String_Long]+1;
			int level = matcher.GetCaptureGroupEnding(group) - matcher.GetCaptureGroupBeginning(group);
			BlockLevel = level;
			SetContext(Context_String);
			break;
		}
		case FIN_Lua_Token_LongBracketClose: {
			int group = context.GroupMap[FIN_Lua_Token_LongBracketClose]+1;
			int level = matcher.GetCaptureGroupEnding(group) - matcher.GetCaptureGroupBeginning(group);
			if (*BlockLevel == level) {
				BlockLevel.Reset();
				SetContext(Context_Main);
			}
			break;
		}
		default: ;
	}

 	return FFINLuaToken(token, textRange);
}
UE_ENABLE_OPTIMIZATION_SHIP