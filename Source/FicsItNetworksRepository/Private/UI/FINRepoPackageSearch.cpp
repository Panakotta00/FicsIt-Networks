#include "UI/FINRepoPackageSearch.h"

#include "Widgets/Input/SSearchBox.h"

const FName FFINRepoPackageSearchStyle::TypeName(TEXT("FFINRepoPackageSearchStyle"));

void FFINRepoPackageSearchStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	ListStyle.GetResources(OutBrushes);
	OutBrushes.Add(&Background);
}

const FFINRepoPackageSearchStyle& FFINRepoPackageSearchStyle::GetDefault() {
	static FFINRepoPackageSearchStyle style;
	return style;
}

void SFINRepoPackageSearch::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;

	ChildSlot[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(&Style->Background)
		.Content()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)[
				SNew(SSearchBox)
				.Style(&Style->SearchBoxStyle)
				.DelayChangeNotificationsWhileTyping(true)
				.OnTextChanged_Lambda([this](const FText& Text) {
					Query.Query = Text.ToString();
					PackageList->RefreshSearch();
				})
			]
			+SVerticalBox::Slot()
			.FillHeight(1)[
				SAssignNew(PackageList, SFINRepoPackageList)
				.Style(&Style->ListStyle)
				.Query_Lambda([this]() {
					return Query;
				})
				.OnPackageClick(InArgs._OnPackageClick)
			]
		]
	];
}
