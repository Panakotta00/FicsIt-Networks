#pragma once

#include "FINLogViewer.h"
#include "FINStyle.h"
#include "FINTextDecorators.h"
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
	FButtonStyle UnderlineStyleValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FButtonStyle UnderlineStyleInvalid;

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
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINLuaCodeEditorStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

class FICSITNETWORKS_API FFINTabRun : public ISlateRun, public TSharedFromThis<FFINTabRun> {
protected:
	FRunInfo RunInfo;
	TSharedRef<const FString> Text;
	FTextBlockStyle Style;
	FTextRange Range;
	int32 TabWidthInSpaces;

	int32 CharsTillTabStep = 0;
	
public:
	static TSharedRef<FFINTabRun> Create(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FTextBlockStyle& Style, const FTextRange& InRange, int32 InTabWith);

	FFINTabRun(const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FTextBlockStyle& InStyle, const FTextRange& InRange, int32 InTabWidth);

	virtual FTextRange GetTextRange() const override;
	virtual void SetTextRange(const FTextRange& Value) override;
	virtual int16 GetBaseLine(float Scale) const override;
	virtual int16 GetMaxHeight(float Scale) const override;
	virtual FVector2D Measure(int32 StartIndex, int32 EndIndex, float Scale, const FRunTextContext& TextContext) const override;
	virtual int8 GetKerning(int32 CurrentIndex, float Scale, const FRunTextContext& TextContext) const override;
	virtual TSharedRef<ILayoutBlock> CreateBlock(int32 StartIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr<IRunRenderer>& Renderer) override;
	virtual int32 GetTextIndexAt(const TSharedRef<ILayoutBlock>& Block, const FVector2D& Location, float Scale, ETextHitPoint* const OutHitPoint) const override;
	virtual FVector2D GetLocationAt(const TSharedRef<ILayoutBlock>& Block, int32 Offset, float Scale) const override;
	virtual void BeginLayout() override;
	virtual void EndLayout() override;
	virtual void Move(const TSharedRef<FString>& NewText, const FTextRange& NewRange) override;
	virtual TSharedRef<IRun> Clone() const override;
	virtual void AppendTextTo(FString& Text) const override;
	virtual void AppendTextTo(FString& Text, const FTextRange& Range) const override;
	virtual const FRunInfo& GetRunInfo() const override;
	virtual ERunAttributes GetRunAttributes() const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FTextArgs& TextArgs, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual const TArray<TSharedRef<SWidget>>& GetChildren() override;
	virtual void ArrangeChildren(const TSharedRef<ILayoutBlock>& Block, const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	static int32 CalcCharacterTillNextTabStop(const FString& Text, int32 Offset, int32 TabWidth);
};

class FICSITNETWORKS_API FFINLuaSyntaxHighlighterTextLayoutMarshaller : public FPlainTextLayoutMarshaller {
public:
	virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;
	virtual bool RequiresLiveUpdate() const override;

	FFINLuaSyntaxHighlighterTextLayoutMarshaller(const FFINLuaCodeEditorStyle* InLuaSyntaxTextStyle, FFINReflectionReferenceDecorator::FOnNavigate NavigateDelegate);

protected:
	void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines);

	const FFINLuaCodeEditorStyle* SyntaxTextStyle;
	FFINReflectionReferenceDecorator::FOnNavigate NavigateDelegate;
};

class SFINLuaCodeEditor : public SBorder {
public:
	typedef FFINReflectionReferenceDecorator::FOnNavigate FOnNavigateReflection;
	
private:
	TSharedPtr<FFINLuaSyntaxHighlighterTextLayoutMarshaller> SyntaxHighlighter;

	TSharedPtr<FSlateTextLayout> TextLayout;

	const FFINLuaCodeEditorStyle* Style = nullptr;

public:
	TSharedPtr<SMultiLineEditableText> TextEdit;
	TSharedPtr<SScrollBar> HScrollBar;
	TSharedPtr<SScrollBar> VScrollBar;
	
	SLATE_BEGIN_ARGS(SFINLuaCodeEditor) :
		_Style(&FFINStyle::Get().GetWidgetStyle<FFINLuaCodeEditorStyle>("LuaCodeEditor")) {}
	SLATE_STYLE_ARGUMENT(FFINLuaCodeEditorStyle, Style)
	SLATE_ATTRIBUTE(FMargin, Padding)
	SLATE_EVENT(FOnTextChanged, OnTextChanged)
    SLATE_EVENT(FOnTextCommitted, OnTextCommitted)
    SLATE_EVENT(FOnNavigateReflection, OnNavigateReflection)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	void NavigateToLine(int64 LineNumber);
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

	UPROPERTY(BlueprintAssignable)
	FFINNavigateReflection OnNavigateReflection;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FFINLuaCodeEditorStyle Style;

	UFINLuaCodeEditor() : Style(FFINStyle::Get().GetWidgetStyle<FFINLuaCodeEditorStyle>("LuaCodeEditor")) {}

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

	UFUNCTION(BlueprintCallable)
	void NavigateToLine(int64 LineNumber);
};