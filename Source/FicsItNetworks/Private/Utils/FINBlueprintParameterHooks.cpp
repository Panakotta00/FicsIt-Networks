#include "Utils/FINBlueprintParameterHooks.h"

#include "FGPlayerController.h"
#include "FINConfigurationStruct.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "UI/FGGameUI.h"
#include "Utils/FINUtils.h"
#include "Widgets/Input/SEditableTextBox.h"

const FName AFINBlueprintHologram_List_NameColumn = FName("Name");	
const FName AFINBlueprintHologram_List_ValueColumn = FName("Value");

AFINBlueprintHologram::AFINBlueprintHologram() {
	PopupClass = ConstructorHelpers::FClassFinder<UFGPopupWidget>(TEXT("/Game/FactoryGame/Interface/UI/InGame/Widget_Popup.Widget_Popup_C")).Class;
	PopupContentClass = ConstructorHelpers::FClassFinder<UFINBlueprintParameterPopup>(TEXT("/FicsItNetworks/UI/Widget_FIN_BlueprintParameterPopup.Widget_FIN_BlueprintParameterPopup_C")).Class;
}

void AFINBlueprintHologram::ShowPropertyDialog() {
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
		UFINBlueprintParameterPopup* PopupContent = CreateWidget<UFINBlueprintParameterPopup>(GetWorld()->GetFirstPlayerController(), PopupContentClass);
		PopupContent->Hologram = this;

		FPopupClosed PopupClosedDelegate;
		PopupClosedDelegate.BindDynamic(this, &AFINBlueprintHologram::PopupClosed);
		UFGBlueprintFunctionLibrary::AddPopupWithContent(
			GetWorld()->GetFirstPlayerController(),
			FText::FromString(TEXT("FicsIt-Networks Blueprint Parameters")),
			FText(),
			PopupClosedDelegate,
			PopupContent,
			PID_OK_CANCEL,
			nullptr);
	});

	/*auto Hud = GetWorld()->GetFirstPlayerController()->GetHUD();
	auto FGHud = static_cast<AFGHUD*>(Hud);
	if(IsValid(FGHud)) {
		FGHud->OpenInteractUI(UTestWindow::StaticClass(), this);
	}*/
}

bool AFINBlueprintHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if(Parameters.Num() > 0) {
		if(!bConfigured) {
			ShowPropertyDialog();
			return false;
		}
		return true;
	}

	return Super::DoMultiStepPlacement(isInputFromARelease);
}

AActor* AFINBlueprintHologram::Construct(TArray<AActor*>& out_children, FNetConstructionID NetConstructionID) {
	AActor* Construct = Super::Construct(out_children, NetConstructionID);

	if(Parameters.Num() > 0 && bAccepted) {
		for (AActor* Child : out_children) {
			if (Child->Implements<UFINNetworkComponent>()) {
				FString Nick = IFINNetworkComponent::Execute_GetNick(Child);
				Nick = UFINUtils::InterpolateString(Nick, Parameters, false);
				IFINNetworkComponent::Execute_SetNick(Child, Nick);
			}
			for (UActorComponent* Component : Child->GetComponents()) {
				if (Component->Implements<UFINNetworkComponent>()) {
					FString Nick = IFINNetworkComponent::Execute_GetNick(Component);
					Nick = UFINUtils::InterpolateString(Nick, Parameters, false);
					IFINNetworkComponent::Execute_SetNick(Component, Nick);
				}
			}
		}
	}
	
	return Construct;
}

void AFINBlueprintHologram::BeginPlay() {
	Super::BeginPlay();

	TArray<AFGBuildable*> Keys;
	mBuildableToNewRoot.GetKeys(Keys);
	
	for (AFGBuildable* Child : Keys) {
		if (Child->Implements<UFINNetworkComponent>()) {
			FString Nick = IFINNetworkComponent::Execute_GetNick(Child);
			UFINUtils::VariablesFormString(Nick, Parameters);
		}
		for (UActorComponent* Component : Child->GetComponents()) {
			if (Component->Implements<UFINNetworkComponent>()) {
				FString Nick = IFINNetworkComponent::Execute_GetNick(Component);
				UFINUtils::VariablesFormString(Nick, Parameters);
			}
		}
	}

	Parameters.KeySort(TLess<FString>());
}

void AFINBlueprintHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	if(Parameters.Num() <= 0 || !bConfigured) {
		Super::SetHologramLocationAndRotation(hitResult);	
	}
}

bool AFINBlueprintHologram::TrySnapToActor(const FHitResult& hitResult) {
	if(Parameters.Num() <= 0 || !bConfigured) {
		return Super::TrySnapToActor(hitResult);
	}
	return false;
}

void AFINBlueprintHologram::PlaceHologram() {
	auto cp = static_cast<AFGCharacterPlayer*>(mConstructionInstigator);
	cp->GetBuildGun()->Server_PrimaryFire_Implementation();
	cp->GetBuildGun()->Server_PrimaryFire_Implementation();
}

void AFINBlueprintHologram::PopupClosed(bool bConfirmed) {
	bConfigured = true;
	bAccepted = bConfirmed;
	PlaceHologram();
}

void AFINBlueprintParameterHooks::Init() {
	Super::Init();

	if(FFINConfigurationStruct::GetActiveConfig(GetWorld()).Blueprints.EnableParametricBlueprints) {
		const UClass* BP_BuildBlueprint = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Buildable/Factory/BlueprintBuildable/Build_Blueprint.Build_Blueprint_C"));//FindObject<UClass>(nullptr, TEXT("Build_Blueprint.Build_Blueprint_C"), true);

		if (IsValid(BP_BuildBlueprint)) {
			BP_BuildBlueprint->GetDefaultObject<AFGBuildable>()->mHologramClass = AFINBlueprintHologram::StaticClass(); 
		}
	}
}

void SFINBlueprintParameterRow::Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FString& InVariableName, AFINBlueprintHologram* InHologram) {
	VariableName = InVariableName;
	Hologram = InHologram;
	check(Hologram);
	
	FSuperRowType::Construct(InArgs, OwnerTable);
}

TSharedRef<SWidget> SFINBlueprintParameterRow::GenerateWidgetForColumn(const FName& ColumnName) {
	if(ColumnName == AFINBlueprintHologram_List_NameColumn) {
		return SNew(SEditableTextBox)
			.OnKeyDownHandler_Lambda([this](const FGeometry& Geom, const FKeyEvent& Key) {
				if(Key.GetKey() == EKeys::Tab) {
					return FReply::Handled();
				}
				return FReply::Unhandled();
			})
			.Text(FText::FromString(VariableName))
			.IsReadOnly(true);
	}
	if(ColumnName == AFINBlueprintHologram_List_ValueColumn) {
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SEditableTextBox)
			.Text(FText::FromString(Hologram->Parameters[VariableName]))
			.OnKeyDownHandler_Lambda([this](const FGeometry& Geom, const FKeyEvent& Key) {
				if(Key.GetKey() == EKeys::Tab) {
					return FReply::Handled();
				}
				return FReply::Unhandled();
			})
			.OnTextCommitted_Lambda([this](const FText& Text, const ETextCommit::Type CommitType){
				Hologram->Parameters[VariableName] = Text.ToString();
			})
		];
	}
	return SNew(SBorder);
}

void UFINBlueprintParameterPopup::NativeConstruct() {
	Super::NativeConstruct();

	//Hologram = Cast<AFINBlueprintHologram>(mInstigator);
	for (const TPair<FString, FString>& Variable : Hologram->Parameters) {
		Rows.Add(MakeShared<FString>(Variable.Key));
	}

	UOverlaySlot* OSlot = Cast<UOverlaySlot>(Slot->Parent->Slot);
	OSlot->SetPadding(FMargin(0));
	OSlot->SetHorizontalAlignment(HAlign_Fill);
	OSlot->SetVerticalAlignment(VAlign_Fill);
}

TSharedRef<SWidget> UFINBlueprintParameterPopup::RebuildWidget() {
	return SNew(SBox)
		.MinDesiredWidth(FOptionalSize(500))
		.MinDesiredHeight(FOptionalSize(500))
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)[
			SNew(SListView<TSharedRef<FString>>)
			.ItemHeight(24)
			.ListItemsSource(&Rows)
			.OnGenerateRow_Lambda([this](TSharedRef<FString> Item, TSharedRef<STableViewBase> const& OwnerTable) {
				return SNew(SFINBlueprintParameterRow, OwnerTable, *Item, Hologram);
			})
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column(AFINBlueprintHologram_List_NameColumn)
					.DefaultLabel(FText::FromString(TEXT("Name")))
					.ManualWidth(100.0f)
				+SHeaderRow::Column(AFINBlueprintHologram_List_ValueColumn)
					.DefaultLabel(FText::FromString(TEXT("Value")))
					.FillWidth(1.0f)
			)
		];
}
