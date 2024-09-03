#include "UI/FINRepoPackageCard.h"

#include "FINRepoModel.h"

const FName FFINRepoPackageCardStyle::TypeName(TEXT("FFINRepoPackageCardStyle"));

void FFINRepoPackageCardStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	FSlateWidgetStyle::GetResources(OutBrushes);

	TitleStyle.GetResources(OutBrushes);
}

const FFINRepoPackageCardStyle& FFINRepoPackageCardStyle::GetDefault() {
	static FFINRepoPackageCardStyle Style;
	return Style;
}

void SFINRepoPackageCard::Construct(const FArguments& InArgs, const FFINRepoPackageCard& Card) {
	Style = InArgs._Style;

	ChildSlot[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()[
			SNew(STextBlock)
			.Text(FText::FromString(Card.Name))
			.TextStyle(&Style->TitleStyle)
		]
		+SVerticalBox::Slot().AutoHeight()[
			SNew(STextBlock)
			.Text(FText::FromString(Card.ShortDescription))
			.TextStyle(&Style->DescriptionStyle)
		]
	];
}
