#include "FINReflectionUIStyle.h"

const FName FFINReflectionUIStyleStruct::TypeName(TEXT("FFINReflectionUIStyleStruct"));

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
