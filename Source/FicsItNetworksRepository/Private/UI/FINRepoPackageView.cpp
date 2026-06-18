#include "UI/FINRepoPackageView.h"

#include "FicsItNetworksRepository.h"
#include "FINRepoEndpoint.h"
#include "FINRepoModel.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Text/STextBlock.h"

const FName FFINRepoEEPROMBoxStyle::TypeName(TEXT("FFINRepoEEPROMBoxStyle"));

void FFINRepoEEPROMBoxStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	FSlateWidgetStyle::GetResources(OutBrushes);

	TitleStyle.GetResources(OutBrushes);
	ButtonStyle.GetResources(OutBrushes);
	ButtonTextStyle.GetResources(OutBrushes);
	DescriptionStyle.GetResources(OutBrushes);

	OutBrushes.Add(&Border);
}

const FFINRepoEEPROMBoxStyle& FFINRepoEEPROMBoxStyle::GetDefault() {
	static FFINRepoEEPROMBoxStyle Style;
	return Style;
}

const FName FFINRepoPackageViewStyle::TypeName(TEXT("FFINRepoPackageViewStyle"));

void FFINRepoPackageViewStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	TitleStyle.GetResources(OutBrushes);
	TitleVersionStyle.GetResources(OutBrushes);
	SidebarTitleTextStyle.GetResources(OutBrushes);
	TagHashtagTextStyle.GetResources(OutBrushes);
	TagTextStyle.GetResources(OutBrushes);
	SidebarVersionTextStyle.GetResources(OutBrushes);
	SidebarVersionTextStyleThis.GetResources(OutBrushes);
	SidebarAuthorTextStyle.GetResources(OutBrushes);
	EEPROMHeaderStyle.GetResources(OutBrushes);
	EEPROMBoxStyle.GetResources(OutBrushes);

	OutBrushes.Add(&TagBorder);
	OutBrushes.Add(&SidebarBackground);
}

const FFINRepoPackageViewStyle& FFINRepoPackageViewStyle::GetDefault() {
	static FFINRepoPackageViewStyle Style;
	return Style;
}

void SFINRepoPackageView::Construct(const FArguments& InArgs, const FString& PackageID, const FString& Version) {
	Style = InArgs._Style;
	OnLoadCode = InArgs._OnLoadCode;

	ChangePackage(PackageID, Version);
}

void SFINRepoPackageView::ChangePackage(const FString& PackageID, const FString& Version) {
	TWeakPtr<SFINRepoPackageView> weakSelf = SharedThis(this);
	auto endpoint = GEngine->GetEngineSubsystem<UFINRepoEndpoint>();
	Request = endpoint->CreateGetPackageRequest(PackageID, Version);
	Request->OnProcessRequestComplete().BindLambda([endpoint, weakSelf](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
		TSharedPtr<SFINRepoPackageView> self = weakSelf.Pin();
		if (!self.IsValid()) return;

		TOptional<TTuple<FFINRepoPackage, FFINRepoVersion>> packageOpt = endpoint->PackageFromResponse(Response);
		if (packageOpt.IsSet()) {
			auto [package, version] = *packageOpt;

			TSharedPtr<SWrapBox> TagsBox;
			TSharedPtr<SVerticalBox> AuthorsBox;
			TSharedPtr<SVerticalBox> VersionsBox;
			TSharedPtr<SWrapBox> EEPROMsBox;
			self->ChildSlot[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(2)
				.Padding(20)[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight()[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock)
							.TextStyle(&self->Style->TitleStyle)
							.Text(FText::FromString(package.Name))
						]
						+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20,0,0, 3)[
							SNew(STextBlock)
							.TextStyle(&self->Style->TitleVersionStyle)
							.Text(FText::FromString(FString::Printf(TEXT("v%s"), *version.Version)))
						]
					]
					+SVerticalBox::Slot().AutoHeight().Padding(10, 5)[
						SAssignNew(TagsBox, SWrapBox)
						.UseAllottedSize(true)
						.InnerSlotPadding(FVector2D(5))
					]
					+SVerticalBox::Slot().FillHeight(1).Padding(0, 20)[
						SNew(SScrollBox)
						+SScrollBox::Slot()[
							SNew(STextBlock)
							.TextStyle(&self->Style->EEPROMBoxStyle.DescriptionStyle)
							.Text(FText::FromString(package.Readme))
							.AutoWrapText(true)
						]
					]
					+SVerticalBox::Slot().AutoHeight()[
						SNew(STextBlock)
                        .TextStyle(&self->Style->EEPROMHeaderStyle)
                        .Text(FText::FromString(TEXT("EEPROMs")))
					]
					+SVerticalBox::Slot().AutoHeight().Padding(20)[
						SAssignNew(EEPROMsBox, SWrapBox)
						.HAlign(HAlign_Fill)
						.Orientation(EOrientation::Orient_Horizontal)
						.UseAllottedSize(true)
						.InnerSlotPadding(FVector2D(10.0))
					]
				]
				+SHorizontalBox::Slot()
				.AutoWidth()[
					SNew(SBorder)
					.BorderImage(&self->Style->SidebarBackground)
					.Padding(20)
					.Content()[
						SNew(SVerticalBox)
						+SVerticalBox::Slot().AutoHeight()[
							SNew(STextBlock)
							.TextStyle(&self->Style->SidebarTitleTextStyle)
							.Text(FText::FromString(TEXT("Authors")))
						]
						+SVerticalBox::Slot().AutoHeight().Padding(20, 0, 20, 20)[
							SAssignNew(AuthorsBox, SVerticalBox)
						]
						+SVerticalBox::Slot().AutoHeight()[
							SNew(STextBlock)
							.TextStyle(&self->Style->SidebarTitleTextStyle)
							.Text(FText::FromString(TEXT("Versions")))
						]
						+SVerticalBox::Slot().AutoHeight().Padding(20, 0, 20, 20)[
							SAssignNew(VersionsBox, SVerticalBox)
						]
					]
				]
			];

			for (const FString& tag : package.Tags) {
				TagsBox->AddSlot()[
					SNew(SBorder)
					.BorderImage(&self->Style->TagBorder)
					.Padding(10, 5)
					.Content()[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock)
							.TextStyle(&self->Style->TagHashtagTextStyle)
							.Text(FText::FromString(TEXT("#")))
						]
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock)
							.TextStyle(&self->Style->TagTextStyle)
							.Text(FText::FromString(tag))
						]
					]
				];
			}
			for (const FString& author : package.Authors) {
				AuthorsBox->AddSlot().AutoHeight()[
					SNew(STextBlock)
					.TextStyle(&self->Style->SidebarAuthorTextStyle)
					.Text(FText::FromString(author))
				];
			}
			for (const FFINRepoVersion& v : package.Versions) {
				const FTextBlockStyle* textStyle = &self->Style->SidebarVersionTextStyle;
				if (v.Version == version.Version) {
					textStyle = &self->Style->SidebarVersionTextStyleThis;
				}
				VersionsBox->AddSlot().AutoHeight()[
					SNew(STextBlock)
					.TextStyle(textStyle)
					.Text(FText::FromString(v.Version))
				];
			}
			for (const auto& eeprom : version.EEPROMs) {
				FString name = eeprom.Name;
				EEPROMsBox->AddSlot().VAlign(VAlign_Fill)[
					SNew(SBox)
					.MaxDesiredWidth(400)
					.Content()[
						SNew(SBorder)
						.BorderImage(&self->Style->EEPROMBoxStyle.Border)
						.Padding(20, 5)
						.Content()[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[
									SNew(STextBlock)
									.TextStyle(&self->Style->EEPROMBoxStyle.TitleStyle)
									.Text(FText::FromString(eeprom.Title))
								]
								+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).Padding(5)[
									SNew(SButton)
									.ButtonStyle(&self->Style->EEPROMBoxStyle.ButtonStyle)
									.TextStyle(&self->Style->EEPROMBoxStyle.ButtonTextStyle)
									.Text(FText::FromString("Copy"))
									.OnClicked_Lambda([endpoint, package, version, eeprom]() {
										endpoint->GetEEPROMContent(package.ID, version.Version, eeprom.Name)->AddLambda([](TOptional<FString> content) {
											if (content) {
												FPlatformApplicationMisc::ClipboardCopy(**content);
											}
										});
										return FReply::Handled();
									})
								]
								+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).Padding(5)[
									SNew(SButton)
									.ButtonStyle(&self->Style->EEPROMBoxStyle.ButtonStyle)
									.TextStyle(&self->Style->EEPROMBoxStyle.ButtonTextStyle)
									.Text(FText::FromString("Load"))
									.OnClicked_Lambda([weakSelf, endpoint, package, version, eeprom]() {
										endpoint->GetEEPROMContent(package.ID, version.Version, eeprom.Name)->AddLambda([weakSelf](TOptional<FString> content) {
											TSharedPtr<SFINRepoPackageView> self = weakSelf.Pin();
											if (content && self) {
												self->OnLoadCode.ExecuteIfBound(**content);
											}
										});
										return FReply::Handled();
									})
								]
							]
							+SVerticalBox::Slot().AutoHeight().Padding(5)[
								SNew(STextBlock)
								.TextStyle(&self->Style->EEPROMBoxStyle.DescriptionStyle)
								.Text(FText::FromString(eeprom.Description))
								.AutoWrapText(true)
							]
						]
					]
				];
			}
		} else {
			// TODO: Display Error
		}
	});
	Request->ProcessRequest();
}
