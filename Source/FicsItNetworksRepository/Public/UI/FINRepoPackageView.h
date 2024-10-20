#pragma once

#include "CoreMinimal.h"
#include "AppStyle.h"
#include "FINRepoStyle.h"
#include "SCompoundWidget.h"
#include "SlateWidgetStyle.h"
#include "SlateWidgetStyleAsset.h"
#include "Styling/SlateTypes.h"
#include "FINRepoPackageView.generated.h"

class IHttpRequest;

USTRUCT(BlueprintType)
struct FFINRepoEEPROMBoxStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFINRepoEEPROMBoxStyle() :
		TitleStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		ButtonStyle(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button")),
		ButtonTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		DescriptionStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoEEPROMBoxStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TitleStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FButtonStyle ButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle ButtonTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle DescriptionStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Border;
};

USTRUCT(BlueprintType)
struct FFINRepoPackageViewStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFINRepoPackageViewStyle() :
		TitleStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		TitleVersionStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		SidebarTitleTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		TagHashtagTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		TagTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		SidebarVersionTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		SidebarVersionTextStyleThis(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		SidebarAuthorTextStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")),
		EEPROMHeaderStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINRepoPackageViewStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush TagBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TitleStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TitleVersionStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TagHashtagTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TagTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush SidebarBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle SidebarTitleTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle SidebarVersionTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle SidebarVersionTextStyleThis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle SidebarAuthorTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle EEPROMHeaderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINRepoEEPROMBoxStyle EEPROMBoxStyle;
};

class SFINRepoPackageView : public SCompoundWidget {
public:
	DECLARE_DELEGATE_OneParam(FLoadCodeEvent, const FString&)

	SLATE_BEGIN_ARGS(SFINRepoPackageView) :
	_Style(&FFINRepoStyle::Get().GetWidgetStyle<FFINRepoPackageViewStyle>("PackageView")) {}
	SLATE_EVENT(FLoadCodeEvent, OnLoadCode)
	SLATE_STYLE_ARGUMENT(FFINRepoPackageViewStyle, Style)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FString& PackageID, const FString& Version);

private:
	const FFINRepoPackageViewStyle* Style = nullptr;

	FLoadCodeEvent OnLoadCode;

	TSharedPtr<IHttpRequest> Request;

	void ChangePackage(const FString& PackageID, const FString& Version);
};
