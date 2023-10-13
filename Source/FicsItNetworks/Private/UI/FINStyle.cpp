#include "UI/FINStyle.h"

#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FFINStyle::Instance = nullptr;

void FFINStyle::Initialize() {
	if (!Instance.IsValid()) {
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FFINStyle::Shutdown() {
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique()); 
	Instance.Reset();
}

FName FFINStyle::GetStyleSetName() {
	static FName StyleSetName(TEXT("MenuStyles"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FFINStyle::Create() {
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FFINStyle::GetStyleSetName(), "/FicsItNetworks/UI/Styles", "/FicsItNetworks/UI/Styles");
	return StyleRef;
}

const ISlateStyle& FFINStyle::Get() {
	Initialize();
	return *Instance;
}
