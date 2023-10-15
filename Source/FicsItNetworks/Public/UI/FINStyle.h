#pragma once

#include "Styling/SlateStyle.h"

class FFINStyle {
public:
	static void Initialize();
	static void Shutdown();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create(); 

	static TSharedPtr<FSlateStyleSet> Instance;
};
