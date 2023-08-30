#include "UI/FINReflectionUIStyle.h"

const FName FFINReflectionUIStyleStruct::TypeName(TEXT("FFINReflectionUIStyleStruct"));
const FName FFINSplitterStyle::TypeName(TEXT("FFINSplitterStyle"));

void FFINSplitterStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	Super::GetResources(OutBrushes);
	
	OutBrushes.Add(&HandleIconBrush);
}

const FFINSplitterStyle& FFINSplitterStyle::GetDefault() {
	static FFINSplitterStyle* Default = nullptr;
	if (!Default) Default = new FFINSplitterStyle();
	return *Default;
}

FFINReflectionUIStyleStruct::FFINReflectionUIStyleStruct() {
	
}

void FFINReflectionUIStyleStruct::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	OutBrushes.Add( &MemberFunc );
	OutBrushes.Add( &ClassFunc );
	OutBrushes.Add( &StaticFunc );
	OutBrushes.Add( &MemberAttrib );
	OutBrushes.Add( &ClassAttrib );
	OutBrushes.Add( &Signal );
}

const FFINReflectionUIStyleStruct& FFINReflectionUIStyleStruct::GetDefault() {
	static FFINReflectionUIStyleStruct* Default = nullptr;
	if (!Default) Default = new FFINReflectionUIStyleStruct();
	return *Default;
}
