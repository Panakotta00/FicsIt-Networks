#pragma once
#include "FINRepoStyle.h"

#include "FINRepoPackageCard.generated.h"

struct FFINRepoPackageCard;

USTRUCT(BlueprintType)
struct FFINRepoPackageCardStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	FFINRepoPackageCardStyle() :
		TitleStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		DescriptionStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoPackageCardStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TitleStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle DescriptionStyle;
};

class SFINRepoPackageCard : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFINRepoPackageCard) :
		_Style(&FFINRepoStyle::Get().GetWidgetStyle<FFINRepoPackageCardStyle>("PackageCard")) {}
		SLATE_STYLE_ARGUMENT(FFINRepoPackageCardStyle, Style)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FFINRepoPackageCard& Card);

private:
	const FFINRepoPackageCardStyle* Style = nullptr;
};
