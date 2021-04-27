#pragma once

#include "Slate.h"
#include "Components/Widget.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"

#include "FINLuaCodeEditor.generated.h"

USTRUCT(BlueprintType)
struct FFINLuaCodeEditorStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	FFINLuaCodeEditorStyle();

	virtual ~FFINLuaCodeEditorStyle() {}

	virtual void GetResources( TArray< const FSlateBrush* >& OutBrushes ) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINLuaCodeEditorStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle NormalTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle StringTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle KeywordTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle NumberTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle BoolTrueTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle BoolFalseTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle CommentTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle OperatorTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle FunctionCallTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle FunctionDeclarationTextStyle;
};

class FICSITNETWORKS_API FFINLuaSyntaxHighlighterTextLayoutMarshaller : public FPlainTextLayoutMarshaller {
public:
	virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;
	virtual bool RequiresLiveUpdate() const override;

	FFINLuaSyntaxHighlighterTextLayoutMarshaller(const FFINLuaCodeEditorStyle* InLuaSyntaxTextStyle);

	static TSharedRef<FFINLuaSyntaxHighlighterTextLayoutMarshaller> Create(const FFINLuaCodeEditorStyle* LuaSyntaxTextStyle);

protected:
	void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines);

	const FFINLuaCodeEditorStyle* SyntaxTextStyle;
};

class SFINLuaCodeEditor : public SCompoundWidget {
private:
	TSharedPtr<FFINLuaSyntaxHighlighterTextLayoutMarshaller> SyntaxHighlighter;

public:
	TSharedPtr<SMultiLineEditableTextBox> TextBox;
	
	SLATE_BEGIN_ARGS(SFINLuaCodeEditor) {}
	SLATE_STYLE_ARGUMENT(FEditableTextBoxStyle, Style)
	SLATE_STYLE_ARGUMENT(FFINLuaCodeEditorStyle, CodeStyle)
	SLATE_ATTRIBUTE( FMargin, Padding )
	SLATE_EVENT( FOnTextChanged, OnTextChanged )
    SLATE_EVENT( FOnTextCommitted, OnTextCommitted )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};

UCLASS()
class UFINLuaCodeEditor : public UWidget {
	GENERATED_BODY()
private:
	TSharedPtr<SFINLuaCodeEditor> CodeEditor;

protected:
	void HandleOnTextChanged(const FText& Text);
	void HandleOnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFINCodeEditorChangedEvent, const FText&, Text);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFINCodeEditorCommittedEvent, const FText&, Text, ETextCommit::Type, CommitMethod);

	UPROPERTY(BlueprintAssignable)
	FOnFINCodeEditorChangedEvent OnTextChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFINCodeEditorCommittedEvent OnTextCommitted;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FFINLuaCodeEditorStyle CodeStyle;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FEditableTextBoxStyle Style;

	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable)
	void SetIsReadOnly(bool bInReadOnly);

	UFUNCTION(BlueprintCallable)
	void SetText(FText InText);

	UFUNCTION(BlueprintCallable)
	FText GetText() const;
};