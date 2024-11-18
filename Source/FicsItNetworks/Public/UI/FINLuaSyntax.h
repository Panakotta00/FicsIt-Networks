#pragma once

#include "CoreMinimal.h"
#include "BaseTextLayoutMarshaller.h"
#include "Regex.h"
#include "TextLayout.h"

struct FFINLuaCodeEditorStyle;

enum EFINLuaToken {
	FIN_Lua_Token_Operator,
	FIN_Lua_Token_Keyword,
	FIN_Lua_Token_Label,

	FIN_Lua_Token_Parenthesis,
	FIN_Lua_Token_ParenthesisSquare,
	FIN_Lua_Token_ParenthesisCurly,
	FIN_Lua_Token_Quote,
	FIN_Lua_Token_Comma,
	FIN_Lua_Token_Access,
	FIN_Lua_Token_Separator,

	FIN_Lua_Token_Nil,
	FIN_Lua_Token_Boolean,
	FIN_Lua_Token_Number,

	FIN_Lua_Token_WhiteSpace,
	FIN_Lua_Token_Tab,
	FIN_Lua_Token_Identifier,

	FIN_Lua_Token_String_Long,
	FIN_Lua_Token_String_Escape,

	FIN_Lua_Token_Comment,
	FIN_Lua_Token_Comment_Long,

	FIN_Lua_Token_LongBracketClose,
};

struct FFINLexerPattern {
	FString Pattern;
	EFINLuaToken Token;
	int Groups = 0;
};

struct FFINLexerRuleSet {
	FRegexPattern Regex = FRegexPattern(TEXT(""));
	TMap<EFINLuaToken, int> GroupMap;
	TMap<int, EFINLuaToken> TokenMap;

	TOptional<TTuple<EFINLuaToken, FTextRange>> FindMatchedToken(FRegexMatcher& Matcher, const FStringView Code) const;
};

struct FFINLuaToken {
	TOptional<EFINLuaToken> Token;
	FTextRange TextRange;

	FFINLuaToken(EFINLuaToken Token, FTextRange TextRange) : Token(Token), TextRange(TextRange) {}
	FFINLuaToken(FTextRange TextRange) : TextRange(TextRange) {}
};

class FFINLuaLexer {
public:
	enum EContext {
		Context_Main,
		Context_String,
		Context_Comment,
	};
private:
	static bool bInitialized;
	static FFINLexerRuleSet MainContext;
	static FFINLexerRuleSet StringContext;
	static FFINLexerRuleSet CommentContext;

	static void Initialize();

	static const FFINLexerRuleSet& GetContext(EContext context);

	EContext CurrentContext;
	TOptional<uint64> BlockLevel;
	TOptional<char> Quote;
	FStringView Code;
	int32 prevEnd = 0;
	int32 offset = 0;
	TOptional<FRegexMatcher> CurrentMatcher;
	bool bFound = false;

	void SetContext(EContext Context);

public:
	void Reset(const FStringView InCode);
	void SetCode(const FStringView InCode);

	EContext GetContext() const {
		return CurrentContext;
	}

	FFINLuaLexer() {
		Initialize();
		Reset(TEXT(""));
	}

	TOptional<FFINLuaToken> NextToken();
};

class FFINLuaSyntaxHighlighter : public FBaseTextLayoutMarshaller {
private:
	const FFINLuaCodeEditorStyle* SyntaxTextStyle;

	FFINLuaLexer Lexer;

public:
	FFINLuaSyntaxHighlighter(const FFINLuaCodeEditorStyle* SyntaxTextStyle) : SyntaxTextStyle(SyntaxTextStyle) {}

	virtual bool RequiresLiveUpdate() const override {
		return true;
	}

	virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;

	virtual void GetText(FString& TargetString, const FTextLayout& SourceTextLayout) override {
		SourceTextLayout.GetAsText(TargetString);
	}
};
