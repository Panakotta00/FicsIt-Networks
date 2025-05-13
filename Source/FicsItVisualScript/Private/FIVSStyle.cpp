#include "FIVSStyle.h"

#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FFIVSStyle::Instance = nullptr;

void FFIVSStyle::Initialize() {
	if (!Instance.IsValid()) {
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FFIVSStyle::Shutdown() {
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FFIVSStyle::GetStyleSetName() {
	static FName StyleSetName(TEXT("FIVSStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FFIVSStyle::Create() {
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(GetStyleSetName(), "/FicsItNetworks/FicsItVisualScript/Styles", "/FicsItNetworks/FicsItVisualScript/Styles");
	return StyleRef;
}

const ISlateStyle& FFIVSStyle::Get() {
	Initialize();
	return *Instance;
}
