#pragma once

#include "Framework/Text/IRichTextMarkupWriter.h"

class FICSITNETWORKS_API FFINPureTextWriter : public IRichTextMarkupWriter {
public:
	virtual void Write(const TArray<FRichTextLine>& InLines, FString& Output) override;
};
