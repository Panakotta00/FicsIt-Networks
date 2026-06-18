#pragma once

#include "CoreMinimal.h"
#include "FINRepoEndpoint.h"
#include "FINRepoPackageCard.h"
#include "FINRepoStyle.h"
#include "FINRepoPackageList.generated.h"

class IHttpRequest;
struct FFINRepoPackageCard;

USTRUCT(BlueprintType)
struct FFINRepoPackageListStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFINRepoPackageListStyle() :
		TableViewStyle(FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView")),
		ScrollBarStyle(FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar")),
		RowStyle(FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row")),
		CardStyle(FFINRepoPackageCardStyle::GetDefault()) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoPackageListStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableViewStyle TableViewStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FScrollBarStyle ScrollBarStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableRowStyle RowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINRepoPackageCardStyle CardStyle;
};

class SFINRepoPackageList : public SCompoundWidget {
public:
	DECLARE_DELEGATE_OneParam(FOnPackageClick, TSharedPtr<FFINRepoPackageCard>);

	SLATE_BEGIN_ARGS(SFINRepoPackageList) :
		_Style(&FFINRepoStyle::Get().GetWidgetStyle<FFINRepoPackageListStyle>("PackageList")),
		_PageSize(10),
		_RemainingScrollDistanceForNextPage(0) {}
		SLATE_STYLE_ARGUMENT(FFINRepoPackageListStyle, Style)
		SLATE_EVENT(FOnPackageClick, OnPackageClick)
		SLATE_ARGUMENT(uint64, PageSize)
		SLATE_ATTRIBUTE(FFINRepoSearchQuery, Query)
		SLATE_ATTRIBUTE(double, RemainingScrollDistanceForNextPage)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void SetQuery(const FFINRepoSearchQuery& InQuery);
	void RefreshSearch();

protected:
	void RequestNexPage();
	void CheckScrollDistance();

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FFINRepoPackageCard> Entry, const TSharedRef<class STableViewBase>& ListView);

private:
	TSharedPtr<SListView<TSharedPtr<FFINRepoPackageCard>>> ListView;

	TAttribute<FFINRepoSearchQuery> Query;
	TArray<TSharedPtr<FFINRepoPackageCard>> PackageCards;
	uint64 Page = 0;
	uint64 PageSize = 10;
	TAttribute<double> RemainingScrollDistanceForNextPage;
	TSharedPtr<IHttpRequest> PageRequest;

	FOnPackageClick OnPackageClick;

	const FFINRepoPackageListStyle* Style = nullptr;
};
