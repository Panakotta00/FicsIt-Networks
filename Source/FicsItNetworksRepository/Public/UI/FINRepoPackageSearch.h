#pragma once

#include "CoreMinimal.h"
#include "FINRepoPackageList.h"
#include "Components/Widget.h"
#include "FINRepoPackageSearch.generated.h"

USTRUCT(BlueprintType)
struct FFINRepoPackageSearchStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFINRepoPackageSearchStyle() :
		SearchBoxStyle(FAppStyle::Get().GetWidgetStyle<FSearchBoxStyle>("SearchBox")),
		ListStyle(FFINRepoPackageListStyle::GetDefault()) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoPackageSearchStyle& GetDefault();

	UPROPERTY(EditAnywhere, Category=Appearance)
	FSearchBoxStyle SearchBoxStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINRepoPackageListStyle ListStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Background;
};

class SFINRepoPackageSearch : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFINRepoPackageSearch) :
		_Style(&FFINRepoStyle::Get().GetWidgetStyle<FFINRepoPackageSearchStyle>("PackageSearch")) {}
		SLATE_STYLE_ARGUMENT(FFINRepoPackageSearchStyle, Style)
		SLATE_EVENT(SFINRepoPackageList::FOnPackageClick, OnPackageClick)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	const FFINRepoPackageSearchStyle* Style = nullptr;

	TSharedPtr<SFINRepoPackageList> PackageList;

	FFINRepoSearchQuery Query;
};
