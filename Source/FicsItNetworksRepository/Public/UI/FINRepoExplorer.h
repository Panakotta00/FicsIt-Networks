#pragma once

#include "CoreMinimal.h"
#include "FINRepoPackageSearch.h"
#include "FINRepoPackageView.h"
#include "SCompoundWidget.h"
#include "SlateWidgetStyle.h"
#include "SlateWidgetStyleAsset.h"
#include "SlateWidgetStyleContainerBase.h"
#include "FINRepoExplorer.generated.h"

USTRUCT(BlueprintType)
struct FFINRepoExplorerStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFINRepoExplorerStyle() :
		SearchStyle(FFINRepoPackageSearchStyle::GetDefault()),
		PackageViewStyle(FFINRepoPackageViewStyle::GetDefault()) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoExplorerStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINRepoPackageSearchStyle SearchStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINRepoPackageViewStyle PackageViewStyle;
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINRepoExplorerWidgetStyle : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINRepoExplorerStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

class SFINRepoExplorer : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFINRepoExplorer) :
		_Style(&FFINRepoStyle::Get().GetWidgetStyle<FFINRepoExplorerStyle>("Explorer")) {}
		SLATE_STYLE_ARGUMENT(FFINRepoExplorerStyle, Style)
		SLATE_EVENT(SFINRepoPackageView::FLoadCodeEvent, OnLoadCode)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	const FFINRepoExplorerStyle* Style = nullptr;
	SFINRepoPackageView::FLoadCodeEvent OnLoadCode;

	TSharedPtr<SBox> PackageBox;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINLoadCodeEvent, const FString&, Code);

UCLASS()
class UFINRepoExplorer : public UWidget {
	GENERATED_BODY()
public:
	UFINRepoExplorer();

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	UPROPERTY(EditAnywhere)
	FFINRepoExplorerStyle Style;

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FFINLoadCodeEvent OnLoadCode;
};
