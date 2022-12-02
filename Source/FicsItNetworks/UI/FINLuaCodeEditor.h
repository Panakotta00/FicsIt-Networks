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
	FTextBlockStyle LineNumberStyle;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FScrollBarStyle ScrollBarStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin HScrollBarPadding;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin VScrollBarPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush BorderImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor BackgroundColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ForegroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin Padding;
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINCodeEditorStyle : public USlateWidgetStyleContainerBase {
public:
	GENERATED_BODY()

public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=( ShowOnlyInnerProperties ))
	FFINLuaCodeEditorStyle Style;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override {
		return static_cast< const struct FSlateWidgetStyle* >( &Style );
	}
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

class SFINLuaCodeEditor : public SBorder {
public:
private:
	TSharedPtr<FFINLuaSyntaxHighlighterTextLayoutMarshaller> SyntaxHighlighter;

	TSharedPtr<FSlateTextLayout> TextLayout;

	const FFINLuaCodeEditorStyle* Style = nullptr;

public:
	TSharedPtr<SMultiLineEditableText> TextEdit;
	TSharedPtr<SScrollBar> HScrollBar;
	TSharedPtr<SScrollBar> VScrollBar;
	
	SLATE_BEGIN_ARGS(SFINLuaCodeEditor) {}
	SLATE_STYLE_ARGUMENT(FFINLuaCodeEditorStyle, Style)
	SLATE_ATTRIBUTE( FMargin, Padding )
	SLATE_EVENT( FOnTextChanged, OnTextChanged )
    SLATE_EVENT( FOnTextCommitted, OnTextCommitted )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	SFINLuaCodeEditor();
	
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget
};

UCLASS()
class UFINLuaCodeEditor : public UWidget {
	GENERATED_BODY()
private:
	TSharedPtr<SFINLuaCodeEditor> CodeEditor;

	UPROPERTY(EditDefaultsOnly)
	FText Text;

	UPROPERTY()
	bool bReadOnly = false;

protected:
	void HandleOnTextChanged(const FText& InText);
	void HandleOnTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFINCodeEditorChangedEvent, const FText&, InText);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFINCodeEditorCommittedEvent, const FText&, InText, ETextCommit::Type, CommitMethod);

	UPROPERTY(BlueprintAssignable)
	FOnFINCodeEditorChangedEvent OnTextChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFINCodeEditorCommittedEvent OnTextCommitted;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FFINLuaCodeEditorStyle Style;

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End UWidget

	UFUNCTION(BlueprintCallable)
	void SetIsReadOnly(bool bInReadOnly);

	UFUNCTION(BlueprintCallable)
	void SetText(FText InText);

	UFUNCTION(BlueprintCallable)
	FText GetText() const;
};