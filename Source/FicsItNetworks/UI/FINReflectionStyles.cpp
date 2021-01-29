#include "FINReflectionStyles.h"

#include "FINReflectionStyles.h"
#include "SlateGameResources.h" 

TSharedPtr<FSlateStyleSet> FFINReflectionStyles::ReflectionStyleInstance = NULL;

void FFINReflectionStyles::Initialize() {
	if (!ReflectionStyleInstance.IsValid()) {
		ReflectionStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*ReflectionStyleInstance);
	}
}

void FFINReflectionStyles::Shutdown() {
	FSlateStyleRegistry::UnRegisterSlateStyle(*ReflectionStyleInstance);
	ensure(ReflectionStyleInstance.IsUnique());
	ReflectionStyleInstance.Reset();
}

FName FFINReflectionStyles::GetStyleSetName() {
	static FName StyleSetName(TEXT("FINStyles"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FFINReflectionStyles::Create() {
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FFINReflectionStyles::GetStyleSetName(), "/Game/FicsItNetworks/UI/Styles", "/Game/FicsItNetworks/UI/Styles");
	return StyleRef;
}

const ISlateStyle& FFINReflectionStyles::Get() {
	return *ReflectionStyleInstance;
}
