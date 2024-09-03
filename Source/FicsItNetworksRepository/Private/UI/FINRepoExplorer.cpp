#include "UI/FINRepoExplorer.h"

#include "SWebBrowser.h"
#include "SWebBrowserView.h"
#include "WebBrowserModule.h"

const FName FFINRepoExplorerStyle::TypeName(TEXT("FFINRepoExplorerStyle"));

void FFINRepoExplorerStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	SearchStyle.GetResources(OutBrushes);
	PackageViewStyle.GetResources(OutBrushes);
}

const FFINRepoExplorerStyle& FFINRepoExplorerStyle::GetDefault() {
	static FFINRepoExplorerStyle Style;
	return Style;
}

void SFINRepoExplorer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	OnLoadCode = InArgs._OnLoadCode;

	FModuleManager::LoadModuleChecked<IWebBrowserModule>("WebBrowser");

	ChildSlot[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1)[
			SNew(SFINRepoPackageSearch)
			.Style(&Style->SearchStyle)
			.OnPackageClick_Lambda([this](TSharedPtr<FFINRepoPackageCard> Card) {
				if (Card) {
					PackageBox->SetContent(SNew(SFINRepoPackageView, Card->ID, Card->Version)
						.Style(&Style->PackageViewStyle)
						.OnLoadCode(OnLoadCode)
					);
				}
			})
		]
		+SHorizontalBox::Slot()
		.FillWidth(3)[
			SAssignNew(PackageBox, SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
		]
	];
}

UFINRepoExplorer::UFINRepoExplorer() {
	Style = FFINRepoStyle::Get().GetWidgetStyle<FFINRepoExplorerStyle>("Explorer");
}

TSharedRef<SWidget> UFINRepoExplorer::RebuildWidget() {
	return SNew(SFINRepoExplorer)
		.Style(&Style)
		.OnLoadCode_Lambda([this](const FString& Code) {
			OnLoadCode.Broadcast(Code);
		});
}
