#pragma once

#include "Framework/Text/ITextDecorator.h"
#include "Framework/Text/SlateHyperlinkRun.h"

class UFIRBase;

class FICSITNETWORKS_API FFINHyperlinkRun : public FSlateHyperlinkRun {
public:
	static TSharedRef<FFINHyperlinkRun> Create(const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FHyperlinkStyle& InStyle, FOnClick NavigateDelegate, FOnGenerateTooltip InTooltipDelegate, FOnGetTooltipText InTooltipTextDelegate, const FTextRange& InRange, bool bCtrlRequired);
	
	virtual TSharedRef<ILayoutBlock> CreateBlock(int32 StartIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr<IRunRenderer>& Renderer) override;

protected:
	FFINHyperlinkRun(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FHyperlinkStyle& InStyle,
		const FOnClick& InNavigateDelegate, const FOnGenerateTooltip& InTooltipDelegate,
		const FOnGetTooltipText& InTooltipTextDelegate, const FTextRange& InRange, bool bCtrlRequired)
		: FSlateHyperlinkRun(
			InRunInfo, InText, InStyle, InNavigateDelegate, InTooltipDelegate, InTooltipTextDelegate, InRange),
		bCtrlRequired(bCtrlRequired) {}

public:
	bool bCtrlRequired = false;
};

class FICSITNETWORKS_API FFINReflectionReferenceDecorator : public ITextDecorator {
public:
	DECLARE_DELEGATE_OneParam(FOnNavigate, UFIRBase*)

	static const FString Id;
	static const FString MetaDataVariantKey;
	static const FString MetaDataTypeKey;
	
	FFINReflectionReferenceDecorator(const FOnNavigate& InNavigateDelegate);
	
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;
	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override;

	static UFIRBase* ReflectionItemFromType(const FString& Variant, const FString& Type);
	static TSharedRef<FSlateHyperlinkRun> CreateRun(const FRunInfo& RunInfo, const TSharedRef<FString>& InOutModelText, const FHyperlinkStyle* Style, FOnNavigate NavigateDelegate, FTextRange ModelRange, bool bCtrlRequired);
	
protected:
	FOnNavigate NavigateDelegate;
};

class FICSITNETWORKS_API FFINEEPROMReferenceDecorator : public ITextDecorator {
public:
	DECLARE_DELEGATE_OneParam(FOnNavigate, int64)

	static const FString Id;
	static const FString MetaDataLineNumberKey;
	
	FFINEEPROMReferenceDecorator(const FOnNavigate& InNavigateDelegate);
	
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;
	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override;

protected:
	FOnNavigate NavigateDelegate;
};
