#include "Editor/FIVSEdGraphViewerStyle.h"

const FName FFIVSEdPinStyle::TypeName = TEXT("FFIVSEdPinStyle");

const FFIVSEdPinStyle& FFIVSEdPinStyle::GetDefault() {
	static FFIVSEdPinStyle Style;
	return Style;
}

void FFIVSEdPinStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	OutBrushes.Add(&ConnectionIcon);
	OutBrushes.Add(&ConnectionIconConnected);
}

const FName FFIVSEdNodeStyle::TypeName = TEXT("FFIVSEdNodeStyle");

const FFIVSEdNodeStyle& FFIVSEdNodeStyle::GetDefault() {
	static FFIVSEdNodeStyle Style;
	return Style;
}

void FFIVSEdNodeStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	OutBrushes.Add(&Background);
	OutBrushes.Add(&Header);
	OutBrushes.Add(&Outline);
	OutBrushes.Add(&ErrorOutline);
	OutBrushes.Add(&PinOutline);

	HeaderTextStyle.GetResources(OutBrushes);
	PinTextStyle.GetResources(OutBrushes);
	DataInputPinStyle.GetResources(OutBrushes);
	DataOutputPinStyle.GetResources(OutBrushes);
	ExecInputPinStyle.GetResources(OutBrushes);
	ExecOutputPinStyle.GetResources(OutBrushes);
}

const FName FFIVSEdGraphViewerStyle::TypeName = TEXT("FFIVSEdGraphViewerStyle");


const FFIVSEdGraphViewerStyle& FFIVSEdGraphViewerStyle::GetDefault() {
	static FFIVSEdGraphViewerStyle* Default = nullptr;
	if (!Default) Default = new FFIVSEdGraphViewerStyle();
	return *Default;
}

void FFIVSEdGraphViewerStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	Super::GetResources(OutBrushes);
	
	OutBrushes.Add(&Background);
	OutBrushes.Add(&SelectionBox);
	NodeStyle.GetResources(OutBrushes);
}
