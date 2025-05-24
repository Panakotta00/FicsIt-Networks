#pragma once

#include "CoreMinimal.h"
#include "FIVSNode.h"
#include "ITextDecorator.h"

class FICSITVISUALSCRIPT_API FFIVSReferenceDecorator : public ITextDecorator {
public:
	DECLARE_DELEGATE_OneParam(FOnNavigate, UFIVSNode*)

	static const FString Id;
	static const FString MetaDataLineNumberKey;

	FFIVSReferenceDecorator(const FOnNavigate& InNavigateDelegate);

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;
	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override;

protected:
	FOnNavigate NavigateDelegate;
};
