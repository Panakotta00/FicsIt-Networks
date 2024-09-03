#include "UI/FINRepoPackageList.h"

#include "FicsItNetworksRepository.h"
#include "FINRepoEndpoint.h"
#include "Logging/StructuredLog.h"

const FName FFINRepoPackageListStyle::TypeName(TEXT("FFINRepoPackageListStyle"));

UE_DISABLE_OPTIMIZATION_SHIP
void FFINRepoPackageListStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	FSlateWidgetStyle::GetResources(OutBrushes);

	TableViewStyle.GetResources(OutBrushes);
	ScrollBarStyle.GetResources(OutBrushes);
	RowStyle.GetResources(OutBrushes);
	CardStyle.GetResources(OutBrushes);
}

const FFINRepoPackageListStyle& FFINRepoPackageListStyle::GetDefault() {
	static FFINRepoPackageListStyle Style;
	return Style;
}

void SFINRepoPackageList::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	PageSize = InArgs._PageSize;
	OnPackageClick = InArgs._OnPackageClick;
	Query = InArgs._Query;
	RemainingScrollDistanceForNextPage = InArgs._RemainingScrollDistanceForNextPage;

	ChildSlot[
		SAssignNew(ListView, SListView<TSharedPtr<FFINRepoPackageCard>>)
		.ListViewStyle(&Style->TableViewStyle)
		.ScrollBarStyle(&Style->ScrollBarStyle)
		.ListItemsSource(&PackageCards)
		.SelectionMode(ESelectionMode::Type::Single)
		.ClearSelectionOnClick(false)
		.OnSelectionChanged_Lambda([this](TSharedPtr<FFINRepoPackageCard> Card, ESelectInfo::Type) {
			bool _ = OnPackageClick.ExecuteIfBound(Card);
		})
		.OnGenerateRow_Raw(this, &SFINRepoPackageList::OnGenerateRow)
	];

	RequestNexPage();
}

void SFINRepoPackageList::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	CheckScrollDistance();
}

void SFINRepoPackageList::SetQuery(const FFINRepoSearchQuery& InQuery) {
	Query = InQuery;
}

void SFINRepoPackageList::RefreshSearch() {
	Page = 0;
	PageRequest.Reset();
	RequestNexPage();
}

void SFINRepoPackageList::RequestNexPage() {
	if (PageRequest.IsValid()) return;

	TSharedPtr<SFINRepoPackageList> self = SharedThis(this);
	auto endpoint = GEngine->GetEngineSubsystem<UFINRepoEndpoint>();
	PageRequest = endpoint->CreateSearchPackagesRequest(Query.Get(), Page, PageSize);
	PageRequest->OnProcessRequestComplete().BindLambda([endpoint, self](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
		TOptional<TArray<FFINRepoPackageCard>> packages = endpoint->PackagesFromResponse(Response);
		if (packages.IsSet()) {
			if (self->Page == 0) {
				self->PackageCards.Empty();
			}
			self->Page += 1;
			if (packages->IsEmpty()) {
				// No further pages to load
			} else {
				self->PackageCards.Reserve(self->PackageCards.Num() + packages->Num());
				for (const FFINRepoPackageCard& card : *packages) {
					self->PackageCards.Add(MakeShared<FFINRepoPackageCard>(card));
				}
				self->PageRequest = nullptr;
			}
			self->ListView->RequestListRefresh();
		} else {
			if (bConnectedSuccessfully && Response.IsValid()) {
				int32 code = Response->GetResponseCode();
				UE_LOGFMT(LogFicsItNetworksRepo, Warning, "Tried search packages but received response code '{Code}'", code);
			} else {
				UE_LOGFMT(LogFicsItNetworksRepo, Warning, "Failed to retreive response for package search!");
			}
			// TODO: Maybe retry after some seconds?
		}
	});
	PageRequest->ProcessRequest();
}

void SFINRepoPackageList::CheckScrollDistance() {
	if (!ListView) return;
	FVector2D distance = ListView->GetScrollDistanceRemaining();
	if (distance.Y < RemainingScrollDistanceForNextPage.Get()) {
		RequestNexPage();
	}
}
UE_ENABLE_OPTIMIZATION_SHIP

TSharedRef<ITableRow> SFINRepoPackageList::OnGenerateRow(TSharedPtr<FFINRepoPackageCard> Entry, const TSharedRef<STableViewBase>& InListView) {
	return SNew(STableRow<TSharedPtr<FFINRepoPackageCard>>, InListView)
	.Style(&Style->RowStyle)
	.Padding(10)
	.Content()[
		SNew(SFINRepoPackageCard, *Entry)
		.Style(&Style->CardStyle)
	];
}
