#include "Utils/FINBlueprintParameterHooks.h"

#include "FGHUD.h"
#include "FGPlayerController.h"
#include "FINConfigurationStruct.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBox.h"

#pragma optimize("", off)

const FName AFINBlueprintHologram_List_NameColumn = FName("Name");	
const FName AFINBlueprintHologram_List_ValueColumn = FName("Value");




void AFINBlueprintHologram::ShowPropertyDialog() {
	
	auto Hud = GetWorld()->GetFirstPlayerController()->GetHUD();
	auto FGHud = static_cast<AFGHUD*>(Hud);
	if(IsValid(FGHud)) {
		FGHud->OpenInteractUI(UTestWindow::StaticClass(), this);
	}
	
}/* */

void UTestWindow::NativeConstruct() {
	mUseMouse = true;
	mDisablePlayerActions = true;
	mDisableBuildGunActions = true;
	mDisablePlayerEquipmentManagement = true;

	mUseKeyboard = true;
	mCaptureInput = true;
	Priority = 1000;
	
	Super::NativeConstruct();

	AFINBlueprintHologram* Holo = static_cast<AFINBlueprintHologram*>(mInteractObject);
	SetProperties(Holo->GetParameters());
}

TSharedRef<SWidget> UTestWindow::RebuildWidget() {
	return SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SScaleBox)
			.Content()[
				SNew(SBorder)
				.Content()[
					SNew(SBox)
						.MinDesiredWidth(300)
						.MinDesiredHeight(200)
						.Content()[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()[
								SNew(SHeader)
								.HAlign(HAlign_Center)
								.Content()[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Blueprint Network Configuration")))
								]
							]
							+SVerticalBox::Slot()
							.FillHeight(1)[
								SNew(SScrollBox)
								+ SScrollBox::Slot()[
									SAssignNew(ListView, SListView<TSharedRef<FFINBlueprintParamData> > )
									.ItemHeight(24)
									.ListItemsSource( &Rows )
									.OnGenerateRow_Lambda([this]( TSharedRef<FFINBlueprintParamData> Item, TSharedRef<STableViewBase> const& OwnerTable) {
										TSharedRef<SFINPropertyRow> Row = SNew(SFINPropertyRow, ListView.ToSharedRef(), Item);
										return Row;
									})
									.SelectionMode(ESelectionMode::Single)
									.HeaderRow(
										SNew(SHeaderRow)
										+ SHeaderRow::Column(AFINBlueprintHologram_List_NameColumn).DefaultLabel(FText::FromString(TEXT("Name")))
										+ SHeaderRow::Column(AFINBlueprintHologram_List_ValueColumn).DefaultLabel(FText::FromString(TEXT("Value")))
									)
								]
							]
							+SVerticalBox::Slot()
							.AutoHeight()[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.FillWidth(1)
								.HAlign(HAlign_Right)[
									SNew(SButton)
									.Text(FText::FromString("OK"))
									.OnClicked_Lambda([this]() {
										Result = EFINBlueprintParameterDialogAnswers::Action_OK; 
										AFINBlueprintHologram* Holo = static_cast<AFINBlueprintHologram*>(mInteractObject);
										ApplyProperties(Holo->GetParameters());
										Holo->ApplyProperties(Rows);
										//auto temp = Holo->GetOwner();
										//AFGBuildGun::Input_PrimaryFire(FInputActionValue());
										Holo->SetConfigured(true);
										Holo->PlaceHologram();
										OnEscapePressed();
										return FReply::Handled();
									})]
								+SHorizontalBox::Slot()
								.AutoWidth()
								.HAlign(HAlign_Left)[
									SNew(SButton)
									.Text(FText::FromString("Cancel"))
									.OnClicked_Lambda([this]() {
										Result = EFINBlueprintParameterDialogAnswers::Action_Cancel;
										AFINBlueprintHologram* Holo = static_cast<AFINBlueprintHologram*>(mInteractObject);
										Holo->SetConfigured(false);
										OnEscapePressed();
										return FReply::Handled();
									})
							]
						]
					]
				]
			]
		];
}

void UTestWindow::SetProperties(TMap<FString, FFINBlueprintParamData> Data) {
	for (auto Row : Data) {
		Rows.Add(TSharedRef<FFINBlueprintParamData>(MakeShared<FFINBlueprintParamData>(Row.Value)));
	}
}

void UTestWindow::ApplyProperties(TMap<FString, FFINBlueprintParamData> Data) {
	for (const auto Row : Rows) {
		Data[Row->Key].Value = Row->Value;
	}
}
/*
	
	*/


void SFINPropertyRow::Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable,
	const TSharedRef<FFINBlueprintParamData>& PropData) {
	ParamData = PropData;
	FSuperRowType::Construct(InArgs, OwnerTable);
}

TSharedRef<SWidget> SFINPropertyRow::GenerateWidgetForColumn(const FName& ColumnName) {
	if(ColumnName == AFINBlueprintHologram_List_NameColumn) {
		return SNew(SBox)
			.VAlign(VAlign_Top)[
				SNew(SEditableTextBox)
				.OnKeyDownHandler_Lambda([this](const FGeometry& Geom, const FKeyEvent& Key) {
					if(Key.GetKey() == EKeys::Tab) {
						return FReply::Handled();
					}
					return FReply::Unhandled();
				})
				.Text( FText::FromString(ParamData->Key))
				.IsReadOnly(true)
			];
	}
	if(ColumnName == AFINBlueprintHologram_List_ValueColumn) {
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SEditableTextBox)
			.Text(FText::FromString(ParamData->Value))
			.IsReadOnly(false)
			.OnKeyDownHandler_Lambda([this](const FGeometry& Geom, const FKeyEvent& Key) {
				if(Key.GetKey() == EKeys::Tab) {
					return FReply::Handled();
				}
				return FReply::Unhandled();
			})
			.OnTextCommitted_Lambda([this](const FText& Text, const ETextCommit::Type CommitType){
				ParamData->Value = Text.ToString();
			})
		];
	}
	return SNew(SBorder);
}

DECLARE_DELEGATE_OneParam(FRegexpReplaceCallback, FRegexMatcher*);

FString preg_replace_callback(FString String, FString Pattern, std::function<FString(FRegexMatcher*)> Callback) {
	FRegexPattern RPattern(Pattern);
	FRegexMatcher RMatcher(RPattern, String);
	FString Result = "";
	int Start = 0;
	while (RMatcher.FindNext()){
		FString Repl = Callback(&RMatcher);
		const int32 Count = RMatcher.GetMatchBeginning() - Start;
		if(Count > 0) {
			Result.Append(String.Mid(Start, Count));
		}
		Result.Append(Repl);
		Start = RMatcher.GetMatchEnding();
	} 
	if(Start < String.Len()) {
		Result.Append(String.Mid(Start));
	}
	return Result;
}

bool AFINBlueprintHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if(PropertiesSet) {
		return true;
	}
	if(Connectors.Num() > 0 && Properties.Num() > 0) {
		if(!Configured) {
			ShowPropertyDialog();
			return false;
		}
		return true;
	}

	return Super::DoMultiStepPlacement(isInputFromARelease);
}

FString AFINBlueprintHologram::ReplFunction(FRegexMatcher* Matcher) {
	FString Prop = Matcher->GetCaptureGroup(1);
	FFINBlueprintParamData* Repl = Properties.Find(Prop);
	if(Repl != nullptr) {
		return Properties[Prop].Value;
	}
	FString n;
	return n;
}

AActor* AFINBlueprintHologram::Construct(TArray<AActor*>& out_children, FNetConstructionID NetConstructionID) {
	AActor* Construct = Super::Construct(out_children, NetConstructionID);

	if(Connectors.Num() > 0 && Properties.Num() > 0) {
		for (auto Child : out_children) {
			UFINAdvancedNetworkConnectionComponent* Comp = dynamic_cast<UFINAdvancedNetworkConnectionComponent*>(Child->GetComponentByClass(
				UFINAdvancedNetworkConnectionComponent::StaticClass()));
			if(IsValid(Comp)) {
				FString Nick = Comp->Execute_GetNick(Comp);
				auto Callback = std::bind(&AFINBlueprintHologram::ReplFunction, this, std::placeholders::_1);
				Nick = preg_replace_callback(Nick, ParameterPattern, Callback);
				Comp->Execute_SetNick(Comp, Nick);
			}
		}
	}
	
	return Construct;
}

void AFINBlueprintHologram::SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
	Super::SpawnChildren(hologramOwner, spawnLocation, hologramInstigator);
}

void AFINBlueprintHologram::BeginPlay() {
	Super::BeginPlay();

	TArray<AFGBuildable*> Keys;
	int KeyCount = mBuildableToNewRoot.GetKeys(Keys);
	Configured = false;
	Connectors.Empty();
	Properties.Empty();
	PropertiesSet = false;
	for (auto Child : Keys) {
		UFINAdvancedNetworkConnectionComponent* Connector = Cast<UFINAdvancedNetworkConnectionComponent>(Child->GetComponentByClass(UFINNetworkConnectionComponent::StaticClass()));
		if(IsValid(Connector)) {
			FString Nick = Connector->Execute_GetNick(Connector);
			FRegexPattern VarPattern(ParameterPattern);
			FRegexMatcher Matcher(VarPattern, Nick);
			if(Matcher.FindNext()) {
				Connectors.Add(Connector);
				do {
					FString Key = Matcher.GetCaptureGroup(1);
					Properties.Add(Key, FFINBlueprintParamData(Key, ""));
				}while(Matcher.FindNext());
			}
		}
	}
	if(Connectors.Num() > 0 && Properties.Num() > 0) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Found Valid Objects"));
	}
}

void AFINBlueprintHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	if(!(Connectors.Num() > 0 && Properties.Num() > 0) || !Configured) {
		Super::SetHologramLocationAndRotation(hitResult);	
	}
}

bool AFINBlueprintHologram::TrySnapToActor(const FHitResult& hitResult) {
	if(!(Connectors.Num() > 0 && Properties.Num() > 0) || !Configured) {
		return Super::TrySnapToActor(hitResult);
	}
	return false;
}

void AFINBlueprintHologram::ApplyProperties(TArray<TSharedRef<FFINBlueprintParamData>> Rows) {
	for (const auto Row : Rows) {
		Properties[Row->Key].Value = Row->Value;
	}
}

void AFINBlueprintHologram::PlaceHologram() {
	auto cp = static_cast<AFGCharacterPlayer*>(mConstructionInstigator);
	cp->GetBuildGun()->Server_PrimaryFire_Implementation();
	cp->GetBuildGun()->Server_PrimaryFire_Implementation();
}
#pragma optimize("", on)

AFINBlueprintParameterHooks::AFINBlueprintParameterHooks() {

	
}


void AFINBlueprintParameterHooks::Init() {
	Super::Init();

	//SUBSCRIBE_METHOD_VIRTUAL(AFGBlueprintHologram::PreHologramPlacement, GetDefault<AFGBlueprintHologram>(), PreHologramPlacementHook);
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBlueprintHologram::Construct, GetDefault<AFGBlueprintHologram>(), ConstructHook);
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBlueprintHologram::PostHologramPlacement, GetDefault<AFGBlueprintHologram>(), PostHologramPlacementHook);

	
	//static ConstructorHelpers::FObjectFinder<AFGBlueprintHologram> Build_BlueprintRef(TEXT(“Blueprint’/Game/FactoryGame/Buildable/Factory/BlueprintBuildable/Build_Blueprint.Build_Blueprint’”));
	if(FFINConfigurationStruct::GetActiveConfig(GetWorld()).Blueprints.EnableParametricBlueprints) {
		const UClass* BP_BuildBlueprint = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Buildable/Factory/BlueprintBuildable/Build_Blueprint.Build_Blueprint_C"));//FindObject<UClass>(nullptr, TEXT("Build_Blueprint.Build_Blueprint_C"), true);
		
		if (IsValid(BP_BuildBlueprint)) {
			BP_BuildBlueprint->GetDefaultObject<AFGBuildable>()->mHologramClass = AFINBlueprintHologram::StaticClass(); 
		}
	}

}
