#pragma once

#include "Framework/Text/ITextDecorator.h"
#include "Framework/Text/SlateHyperlinkRun.h"

class UFINBase;

class FICSITNETWORKS_API FFINReflectionReferenceDecorator : public ITextDecorator {
public:
	DECLARE_DELEGATE_OneParam(FOnNavigate, UFINBase*)

	static const FString Id;
	static const FString MetaDataVariantKey;
	static const FString MetaDataTypeKey;
	
	FFINReflectionReferenceDecorator(const FOnNavigate& InNavigateDelegate);
	
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;
	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override;

	static UFINBase* ReflectionItemFromType(const FString& Variant, const FString& Type);
	static TSharedRef<FSlateHyperlinkRun> CreateRun(const FRunInfo& RunInfo, const TSharedRef<FString>& InOutModelText, const FHyperlinkStyle* Style, FOnNavigate NavigateDelegate, FTextRange ModelRange);
	
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
