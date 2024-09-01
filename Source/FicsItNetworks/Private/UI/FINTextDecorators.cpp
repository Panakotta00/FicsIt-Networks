#include "UI/FINTextDecorators.h"

#include "FicsItReflection.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/DefaultValueHelper.h"
#include "Styling/CoreStyle.h"
#include "Utils/FINUtils.h"

const FString FFINReflectionReferenceDecorator::Id = TEXT("ReflectionReference");
const FString FFINReflectionReferenceDecorator::MetaDataVariantKey = TEXT("Variant");
const FString FFINReflectionReferenceDecorator::MetaDataTypeKey = TEXT("Type");

TSharedRef<FFINHyperlinkRun> FFINHyperlinkRun::Create(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FHyperlinkStyle& InStyle, FOnClick NavigateDelegate, FOnGenerateTooltip InTooltipDelegate, FOnGetTooltipText InTooltipTextDelegate, const FTextRange& InRange, bool bCtrlRequired) {
	return MakeShareable(new FFINHyperlinkRun(InRunInfo, InText, InStyle, NavigateDelegate, InTooltipDelegate, InTooltipTextDelegate, InRange, bCtrlRequired));
}

TSharedRef<ILayoutBlock> FFINHyperlinkRun::CreateBlock(int32 StartIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr<IRunRenderer>& Renderer) {
	TSharedRef<ILayoutBlock> Block = FSlateHyperlinkRun::CreateBlock(StartIndex, EndIndex, Size, TextContext, Renderer);
	/*if (bCtrlRequired) Children.Last()->SetEnabled(TAttribute<bool>::CreateLambda([] {
		return FSlateApplication::Get().GetModifierKeys().IsControlDown();
	}));*/
	if (bCtrlRequired) Children.Last()->SetVisibility(TAttribute<EVisibility>::CreateLambda([]() {
		return FSlateApplication::Get().GetModifierKeys().IsControlDown() ? EVisibility::Visible : EVisibility::HitTestInvisible;
	}));
	return Block;
}

FFINReflectionReferenceDecorator::FFINReflectionReferenceDecorator(const FOnNavigate& InNavigateDelegate) : NavigateDelegate(InNavigateDelegate) {}

bool FFINReflectionReferenceDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const {
	if (RunParseResult.Name != Id) return false;

	const FTextRange& VariantRange = RunParseResult.MetaData[MetaDataVariantKey];
	const FTextRange& TypeRange = RunParseResult.MetaData[MetaDataTypeKey];
	return nullptr != ReflectionItemFromType(UFINUtils::TextRange(Text, VariantRange), UFINUtils::TextRange(Text, TypeRange));
}

TSharedRef<ISlateRun> FFINReflectionReferenceDecorator::Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) {
	FTextRange ModelRange;
	ModelRange.BeginIndex = InOutModelText->Len();
	*InOutModelText += UFINUtils::TextRange(OriginalText, RunParseResult.ContentRange);
	ModelRange.EndIndex = InOutModelText->Len();

	FRunInfo RunInfo(RunParseResult.Name);
	for(const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData) {
		RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid( Pair.Value.BeginIndex, Pair.Value.Len()));
	}
	
	const FHyperlinkStyle* LinkStyle = nullptr;
	if (Style) LinkStyle = &Style->GetWidgetStyle<FHyperlinkStyle>(TEXT("Hyperlink"));

	return CreateRun(RunInfo, InOutModelText, LinkStyle, NavigateDelegate, ModelRange, false);
}

UFIRBase* FFINReflectionReferenceDecorator::ReflectionItemFromType(const FString& Variant, const FString& Type) {
	if (Variant.ToLower().Equals(TEXT("class")) || Variant.ToLower().Equals(TEXT("object")) || Variant.ToLower().Equals(TEXT("trace"))) {
		return FFicsItReflectionModule::Get().FindClass(Type);
	}

	if (Variant.ToLower().Equals(TEXT("struct"))) {
		return FFicsItReflectionModule::Get().FindStruct(Type);
	}

	return nullptr;
}

TSharedRef<FSlateHyperlinkRun> FFINReflectionReferenceDecorator::CreateRun(const FRunInfo& RunInfo, const TSharedRef<FString>& InOutModelText, const FHyperlinkStyle* Style, FOnNavigate NavigateDelegate, FTextRange ModelRange, bool bCtrlRequired) {
	if (!Style)	{
		Style = &FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>(TEXT("Hyperlink"));
	}

	FSlateHyperlinkRun::FOnClick ClickDelegate = FSlateHyperlinkRun::FOnClick::CreateLambda([NavigateDelegate](const FSlateHyperlinkRun::FMetadata& Metadata) {
		UFIRBase* Type = ReflectionItemFromType(Metadata[MetaDataVariantKey], Metadata[MetaDataTypeKey]);
		bool _ = NavigateDelegate.ExecuteIfBound(Type);
	});
	return FFINHyperlinkRun::Create(RunInfo, InOutModelText, *Style, ClickDelegate, FSlateHyperlinkRun::FOnGenerateTooltip(), FSlateHyperlinkRun::FOnGetTooltipText(), ModelRange, bCtrlRequired);
}

const FString FFINEEPROMReferenceDecorator::Id = TEXT("EEPROMReference");
const FString FFINEEPROMReferenceDecorator::MetaDataLineNumberKey = TEXT("LN");

FFINEEPROMReferenceDecorator::FFINEEPROMReferenceDecorator(const FOnNavigate& InNavigateDelegate) : NavigateDelegate(InNavigateDelegate) {}

bool FFINEEPROMReferenceDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const {
	if (RunParseResult.Name != Id) return false;
	int64 LineNumber;
	return FDefaultValueHelper::ParseInt64(UFINUtils::TextRange(Text, RunParseResult.MetaData[MetaDataLineNumberKey]), LineNumber);
}

TSharedRef<ISlateRun> FFINEEPROMReferenceDecorator::Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) {
	FHyperlinkStyle HyperlinkStyle = FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>(TEXT("Hyperlink"));

	FTextRange ModelRange;
	ModelRange.BeginIndex = InOutModelText->Len();
	*InOutModelText += OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.Len());
	ModelRange.EndIndex = InOutModelText->Len();
	
	FRunInfo RunInfo( RunParseResult.Name );
	for(const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData) {
		RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid( Pair.Value.BeginIndex, Pair.Value.Len()));
	}

	return FSlateHyperlinkRun::Create(RunInfo, InOutModelText, HyperlinkStyle, FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
		FString LineNumberText = Metadata[MetaDataLineNumberKey];
		int64 LineNumber;
		FDefaultValueHelper::ParseInt64(LineNumberText, LineNumber);
		bool _ = NavigateDelegate.ExecuteIfBound(LineNumber-1);
	}), FSlateHyperlinkRun::FOnGenerateTooltip(), FSlateHyperlinkRun::FOnGetTooltipText(), ModelRange);
}
