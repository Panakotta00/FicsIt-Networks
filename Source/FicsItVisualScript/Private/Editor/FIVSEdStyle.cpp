#include "Editor/FIVSEdStyle.h"

const FName FFIVSEdStyle::TypeName = TEXT("FFIVSEdStyle");

const FFIVSEdStyle& FFIVSEdStyle::GetDefault() {
	static FFIVSEdStyle* Default = nullptr;
	if (!Default) Default = new FFIVSEdStyle();
	return *Default;
}

void FFIVSEdStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	Super::GetResources(OutBrushes);
	
	OutBrushes.Add(&DataPinIcon);
	OutBrushes.Add(&DataPinConnectedIcon);
}
