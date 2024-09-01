#pragma once

#include "Styling/SlateStyle.h"

class FICSITNETWORKSREPOSITORY_API FFINRepoStyle {
public:
	static void Initialize();
	static void Shutdown();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();

	static TSharedPtr<FSlateStyleSet> Instance;
};
