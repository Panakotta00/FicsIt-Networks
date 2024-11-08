#include "UI/FINLuaCodeEditor.h"

#include "FINLuaSyntax.h"
#include "FINUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "tracy/Tracy.hpp"
#include "UI/FINTextDecorators.h"

const FName FFINLuaCodeEditorStyle::TypeName(TEXT("FFINLuaCodeEditorStyle"));

FFINLuaCodeEditorStyle::FFINLuaCodeEditorStyle() :
	UnderlineStyleValid(FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>(TEXT("Hyperlink")).UnderlineStyle),
	UnderlineStyleInvalid(FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>(TEXT("Hyperlink")).UnderlineStyle) {}

void FFINLuaCodeEditorStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	UnderlineStyleValid.GetResources(OutBrushes);
	UnderlineStyleInvalid.GetResources(OutBrushes);
	LineNumberStyle.GetResources(OutBrushes);
	NormalTextStyle.GetResources(OutBrushes);
	StringTextStyle.GetResources(OutBrushes);
	NumberTextStyle.GetResources(OutBrushes);
	KeywordTextStyle.GetResources(OutBrushes);
	BoolTrueTextStyle.GetResources(OutBrushes);
	BoolFalseTextStyle.GetResources(OutBrushes);
	CommentTextStyle.GetResources(OutBrushes);
	OperatorTextStyle.GetResources(OutBrushes);
	FunctionCallTextStyle.GetResources(OutBrushes);
	FunctionDeclarationTextStyle.GetResources(OutBrushes);
	ScrollBarStyle.GetResources(OutBrushes);
	OutBrushes.Add(&BorderImage);
}

const FFINLuaCodeEditorStyle& FFINLuaCodeEditorStyle::GetDefault() {
	static FFINLuaCodeEditorStyle Instance;
	return Instance;
}

TSharedRef<FFINTabRun> FFINTabRun::Create(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FTextBlockStyle& Style, const FTextRange& InRange, int32 InTabWidth) {
	return MakeShared<FFINTabRun>(InRunInfo, InText, Style, InRange, InTabWidth);
}

FFINTabRun::FFINTabRun(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FTextBlockStyle& InStyle, const FTextRange& InRange, int32 InTabWidth):
	RunInfo(InRunInfo), Text(InText), Style(InStyle), Range(InRange), TabWidthInSpaces(InTabWidth) {}

FTextRange FFINTabRun::GetTextRange() const {
	return Range;
}

void FFINTabRun::SetTextRange(const FTextRange& Value) {
	Range = Value;
}

int16 FFINTabRun::GetBaseLine(float Scale) const  {
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	return FontMeasure->GetBaseline(Style.Font, Scale);
}

int16 FFINTabRun::GetMaxHeight(float Scale) const {
	const TSharedRef< FSlateFontMeasure > FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	return FontMeasure->GetMaxCharacterHeight(Style.Font, Scale);
}

FVector2D FFINTabRun::Measure(int32 StartIndex, int32 EndIndex, float Scale, const FRunTextContext& TextContext) const {
	const TSharedRef< FSlateFontMeasure > FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	float Width = FontMeasure->Measure(TEXT(" "), Style.Font, Scale).X * CharsTillTabStep;
	if (StartIndex == Range.BeginIndex && EndIndex == StartIndex) {
		return FVector2D(0 , GetMaxHeight(Scale));
	}
	return FVector2D(Width , GetMaxHeight(Scale));
}

int8 FFINTabRun::GetKerning(int32 CurrentIndex, float Scale, const FRunTextContext& TextContext) const {
	return 0;
}

TSharedRef<ILayoutBlock> FFINTabRun::CreateBlock(int32 StartIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr<IRunRenderer>& Renderer) {
	return FDefaultLayoutBlock::Create(SharedThis(this), FTextRange(StartIndex, EndIndex), Size, TextContext, Renderer);
}

int32 FFINTabRun::GetTextIndexAt(const TSharedRef<ILayoutBlock>& Block, const FVector2D& Location, float Scale, ETextHitPoint* const OutHitPoint) const {
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FVector2D BlockOffset = Block->GetLocationOffset();
	float Width = FontMeasure->Measure(TEXT(" "), Style.Font, Scale).X * CharsTillTabStep;
	float Offset = BlockOffset.X + Width/2;
	if (Location.X < BlockOffset.X || Location.X > BlockOffset.X + Width) {
		return INDEX_NONE;
	}
	int32 Index = (Location.X < Offset) ? Range.BeginIndex : Range.EndIndex;
	if (OutHitPoint) *OutHitPoint = Index == Range.BeginIndex ? ETextHitPoint::LeftGutter : ETextHitPoint::WithinText;
	return Index;
}

FVector2D FFINTabRun::GetLocationAt(const TSharedRef<ILayoutBlock>& Block, int32 Offset, float Scale) const {
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	float Width = FontMeasure->Measure(TEXT(" "), Style.Font, Scale).X * CharsTillTabStep;
	FVector2D LocalOffset = Block->GetLocationOffset();
	return FVector2D((Offset == 0) ? LocalOffset.X : LocalOffset.X + Width, LocalOffset.Y);
}

void FFINTabRun::BeginLayout() {
	CharsTillTabStep = CalcCharacterTillNextTabStop(*Text, Range.BeginIndex, TabWidthInSpaces);
}

void FFINTabRun::EndLayout() {}

void FFINTabRun::Move(const TSharedRef<FString>& NewText, const FTextRange& NewRange) {
	Text = NewText;
	Range = NewRange;
}

TSharedRef<IRun> FFINTabRun::Clone() const {
	return FFINTabRun::Create(RunInfo, Text, Style, Range, TabWidthInSpaces);
}

void FFINTabRun::AppendTextTo(FString& AppendToText) const {
	AppendToText.Append(**Text + Range.BeginIndex, Range.Len());
}

void FFINTabRun::AppendTextTo(FString& AppendToText, const FTextRange& PartialRange) const {
	check(Range.BeginIndex <= PartialRange.BeginIndex);
	check(Range.EndIndex >= PartialRange.EndIndex);

	AppendToText.Append(**Text + PartialRange.BeginIndex, PartialRange.Len());
}

const FRunInfo& FFINTabRun::GetRunInfo() const {
	return RunInfo;
}

ERunAttributes FFINTabRun::GetRunAttributes() const {
	return ERunAttributes::SupportsText;
}

int32 FFINTabRun::OnPaint(const FPaintArgs& Args, const FTextArgs& TextArgs, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	return LayerId;
}

const TArray<TSharedRef<SWidget>>& FFINTabRun::GetChildren() {
	static TArray<TSharedRef<SWidget>> NoChildren;
	return NoChildren;
}

void FFINTabRun::ArrangeChildren(const TSharedRef<ILayoutBlock>& Block, const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {}

int32 FFINTabRun::CalcCharacterTillNextTabStop(const FString& Text, int32 Offset, int32 TabWidth) {
	int32 LastTab;
	int32 CharsSinceLastTab = Offset;
	if (Text.Left(Offset).FindLastChar('\t', LastTab)) {
		CharsSinceLastTab = (Offset-1) - LastTab;
	}
	int32 FillerChars = CharsSinceLastTab % TabWidth;
	return TabWidth - FillerChars;
}

void SFINLuaCodeEditor::Construct(const FArguments& InArgs) {
	//SyntaxHighlighter = MakeShared<FFINLuaSyntaxHighlighterTextLayoutMarshaller>(InArgs._Style, InArgs._OnNavigateReflection);
	SyntaxHighlighter = MakeShared<FFINLuaSyntaxHighlighter>(InArgs._Style);

	Style = InArgs._Style;

	HScrollBar = SNew(SScrollBar)
				.Style(&InArgs._Style->ScrollBarStyle)
				.Orientation(Orient_Horizontal)
				.Thickness(FVector2D(9.0f, 9.0f));

	VScrollBar = SNew(SScrollBar)
				.Style(&InArgs._Style->ScrollBarStyle)
				.Orientation(Orient_Vertical)
				.Thickness(FVector2D(9.0f, 9.0f));
	
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(&Style->BorderImage)
		.BorderBackgroundColor(Style->BackgroundColor)
		.ForegroundColor(Style->ForegroundColor)
		.Padding(Style->Padding)[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.FillWidth(1)[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				.FillHeight(1)[
					SAssignNew(TextEdit, SMultiLineEditableText)
					.AutoWrapText(false)
					.Margin(0.0f)
					.Marshaller(SyntaxHighlighter)
					.OnTextChanged(InArgs._OnTextChanged)
					.OnTextCommitted(InArgs._OnTextCommitted)
					.TextShapingMethod(ETextShapingMethod::Auto)
					.TextStyle(&InArgs._Style->NormalTextStyle)
					.HScrollBar(HScrollBar)
					.VScrollBar(VScrollBar)
					.CreateSlateTextLayout_Lambda([this](SWidget* InOwningWidget, const FTextBlockStyle& InDefaultTextStyle) {
						TextLayout = FSlateTextLayout::Create(InOwningWidget, InDefaultTextStyle);
						return TextLayout.ToSharedRef();
					})
					.OnIsTypedCharValid_Lambda([](const TCHAR) {
						return true;
					})
					.OnKeyCharHandler_Lambda([this](const FGeometry&, const FCharacterEvent& Event) {
						if (Event.GetCharacter() == '\r') {
							int32 Index = TextEdit->GetCursorLocation().GetLineIndex()-1;
							if (Index >= 0) {
								FString Line;
								TextEdit->GetTextLine(Index, Line);
								FRegexPattern Pattern(TEXT("^\\s*"));
								FRegexMatcher Regex(Pattern, Line);
								if (Regex.FindNext()) {
									TextEdit->InsertTextAtCursor(Regex.GetCaptureGroup(0));
								}
							}
						}
						return FReply::Unhandled();
					})
				]
				+SVerticalBox::Slot()
				.AutoHeight()[
					SNew(SBox)
					.Padding(Style->HScrollBarPadding)[
						HScrollBar.ToSharedRef()
					]
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()[
				SNew(SBox)
				.Padding(Style->VScrollBarPadding)[
					VScrollBar.ToSharedRef()
				]
			]
		]
	);
}

int32 SFINLuaCodeEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	this->ArrangeChildren(AllottedGeometry, ArrangedChildren);

	if(ArrangedChildren.Num() > 0) {
		check( ArrangedChildren.Num() == 1);
		FArrangedWidget& TheChild = ArrangedChildren[0];

		int32 Layer = 0;
		Layer = TheChild.Widget->Paint( Args.WithNewParent(this), TheChild.Geometry, MyCullingRect, OutDrawElements, LayerId + 1, InWidgetStyle, ShouldBeEnabled( bParentEnabled ) );
	}
	
	float LineNumberWidth = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(FString::FromInt(TextLayout->GetLineViews().Num()), Style->LineNumberStyle.Font, 1).X;
	LineNumberWidth += 5.0;
	FGeometry CodeGeometry = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize() - FVector2D(LineNumberWidth, 0), FSlateLayoutTransform(FVector2D(LineNumberWidth, 0)));
	FSlateRect CodeRect = MyCullingRect.ExtendBy(FMargin(LineNumberWidth, 0, 0, 0));

	const float InverseScale = Inverse(AllottedGeometry.Scale);
	int LineNumber = 0;
	OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry));
	for (const FTextLayout::FLineView& LineView : TextLayout->GetLineViews()) {
		++LineNumber;
		const FVector2D LocalLineOffset = LineView.Offset * InverseScale;
		const FSlateRect LineViewRect(AllottedGeometry.GetRenderBoundingRect(FSlateRect(LocalLineOffset * FVector2D(0, 1), LocalLineOffset + (LineView.Size * InverseScale))));
		if ( !FSlateRect::DoRectanglesIntersect(LineViewRect, MyCullingRect)) {
			continue;
		}

		FPaintGeometry LineGeo = AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(TransformPoint(InverseScale, LineView.Offset)));
		FSlateDrawElement::MakeText(OutDrawElements, LayerId++, LineGeo, FText::FromString(FString::FromInt(LineNumber)), Style->LineNumberStyle.Font, ESlateDrawEffect::None, Style->LineNumberStyle.ColorAndOpacity.GetColor(InWidgetStyle));
	}
	OutDrawElements.PopClip();
	
	return LayerId;
}

void SFINLuaCodeEditor::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	float LineNumberWidth = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(FString::FromInt(TextLayout->GetLineViews().Num()), Style->LineNumberStyle.Font, 1).X;
	LineNumberWidth += 5.0;
	FGeometry CodeGeo = AllottedGeometry.MakeChild(AllottedGeometry.Size - FVector2D(LineNumberWidth, 0), FSlateLayoutTransform(FVector2D(LineNumberWidth, 0)));
	ArrangeSingleChild(GSlateFlowDirection, CodeGeo, ArrangedChildren, ChildSlot, FVector2D(1));
}

void SFINLuaCodeEditor::NavigateToLine(int64 LineNumber) {
	FTimerHandle TimerHandle;
	TSharedRef<SFINLuaCodeEditor> Ptr = SharedThis(this);
	GWorld->GetTimerManager().SetTimerForNextTick([Ptr, LineNumber]() {
		Ptr->TextEdit->ScrollTo(FTextLocation(LineNumber));
		FSlateApplication::Get().SetAllUserFocus(Ptr->TextEdit);
		Ptr->TextEdit->SelectText(FTextLocation(LineNumber), FTextLocation(LineNumber));
	});
}

void UFINLuaCodeEditor::HandleOnTextChanged(const FText& InText) {
	Text = InText;
	OnTextChanged.Broadcast(InText);
}

void UFINLuaCodeEditor::HandleOnTextCommitted(const FText& InText, ETextCommit::Type CommitMethod) {
	Text = InText;
	OnTextCommitted.Broadcast(InText, CommitMethod);
}

TSharedRef<SWidget> UFINLuaCodeEditor::RebuildWidget() {
	return SAssignNew(CodeEditor, SFINLuaCodeEditor)
		.Style(&Style)
		.OnTextChanged(BIND_UOBJECT_DELEGATE(FOnTextChanged, HandleOnTextChanged))
		.OnTextCommitted(BIND_UOBJECT_DELEGATE(FOnTextCommitted, HandleOnTextCommitted))
	.OnNavigateReflection_Lambda([this](UFIRBase* Type) {
		OnNavigateReflection.Broadcast(Type);
	});
}

void UFINLuaCodeEditor::ReleaseSlateResources(bool bReleaseChildren) {
	CodeEditor.Reset();
}

void UFINLuaCodeEditor::SetIsReadOnly(bool bInReadOnly) {
	bReadOnly = bInReadOnly;
	if (CodeEditor) CodeEditor->TextEdit->SetIsReadOnly(bInReadOnly);
}

void UFINLuaCodeEditor::SetText(FText InText) {
	Text = InText;
	if (CodeEditor) CodeEditor->TextEdit->SetText(InText);
}

FText UFINLuaCodeEditor::GetText() const {
	return Text;
}

void UFINLuaCodeEditor::NavigateToLine(int64 LineNumber) {
	if (CodeEditor) CodeEditor->NavigateToLine(LineNumber);
}

