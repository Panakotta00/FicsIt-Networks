#include "FIVSTextDecorations.h"

#include "CoreStyle.h"
#include "FINUtils.h"
#include "SlateHyperlinkRun.h"

const FString FFIVSReferenceDecorator::Id = TEXT("EFIVSReference");
const FString FFIVSReferenceDecorator::MetaDataLineNumberKey = TEXT("LN");

FFIVSReferenceDecorator::FFIVSReferenceDecorator(const FOnNavigate& InNavigateDelegate) : NavigateDelegate(InNavigateDelegate) {}

bool FFIVSReferenceDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const {
	if (RunParseResult.Name != Id) return false;
	int64 LineNumber;
	return FDefaultValueHelper::ParseInt64(UFINUtils::TextRange(Text, RunParseResult.MetaData[MetaDataLineNumberKey]), LineNumber);
}

TSharedRef<ISlateRun> FFIVSReferenceDecorator::Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) {
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
		//bool _ = NavigateDelegate.ExecuteIfBound(LineNumber-1);
	}), FSlateHyperlinkRun::FOnGenerateTooltip(), FSlateHyperlinkRun::FOnGetTooltipText(), ModelRange);
}
