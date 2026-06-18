#include "UI/FINRepoStyle.h"

#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"
#include "UI/FINRepoExplorer.h"
#include "UI/FINRepoPackageList.h"
#include "UI/FINRepoPackageSearch.h"
#include "UI/FINRepoPackageView.h"

TSharedPtr<FSlateStyleSet> FFINRepoStyle::Instance = nullptr;

void FFINRepoStyle::Initialize() {
	if (!Instance.IsValid()) {
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FFINRepoStyle::Shutdown() {
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FFINRepoStyle::GetStyleSetName() {
	static FName StyleSetName(TEXT("FINRepoStyles"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FFINRepoStyle::Create() {
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FFINRepoStyle::GetStyleSetName(), "/FicsItNetworks/UI/Repository", "/FicsItNetworks/UI/Repository");

	StyleRef->Set(TEXT("PackageSearch"), StyleRef->GetWidgetStyle<FFINRepoExplorerStyle>("Explorer").SearchStyle);
	StyleRef->Set(TEXT("PackageList"), StyleRef->GetWidgetStyle<FFINRepoPackageSearchStyle>("PackageSearch").ListStyle);
	StyleRef->Set(TEXT("PackageCard"), StyleRef->GetWidgetStyle<FFINRepoPackageListStyle>("PackageList").CardStyle);
	StyleRef->Set(TEXT("PackageView"), StyleRef->GetWidgetStyle<FFINRepoExplorerStyle>("Explorer").PackageViewStyle);

	return StyleRef;
}

const ISlateStyle& FFINRepoStyle::Get() {
	Initialize();
	return *Instance;
}
