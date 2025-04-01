#include "Components/FINArrowModuleHolo.h"

#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "FGBuildGunBuild.h"
#include "FGPlayerController.h"
#include "FicsItNetworksModule.h"
#include "FINArrowModuleBase.h"
#include "MCPBlueprintLibrary.h"
#include "SUniformGridPanel.h"
#include "TimerManager.h"
#include "Components/FINDefaultExtendedHolo.h"
#include "Components/OverlaySlot.h"
#include "Kismet/GameplayStatics.h"
#include "UI/FGGameUI.h"
#include "Widgets/Input/SEditableTextBox.h"


AFINBuildgunHooks::AFINBuildgunHooks() {
}

void AFINBuildgunHooks::BeginPlay() {
	Super::BeginPlay();

	if(!IsRunningDedicatedServer()) {
		auto var = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		AFGCharacterPlayer* AFG = Cast<AFGCharacterPlayer>(var);
		AFG->GetBuildGun()->mOnRecipeSampled.AddUniqueDynamic(this, &AFINBuildgunHooks::OnRecipeSampled);
	}

	
}

void AFINBuildgunHooks::OnRecipeSampled(TSubclassOf<UFGRecipe> Recipe) {
	auto var = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	AFGCharacterPlayer* AFG = Cast<AFGCharacterPlayer>(var);
	AFGBuildGun* BuildGun = AFG->GetBuildGun();
	AActor* Actor = BuildGun->mHitResult.GetActor();
	AFINArrowModuleBase* Module = dynamic_cast<AFINArrowModuleBase*>(Actor);
	if(IsValid(Actor)) {
		UFGBuildGunState* State = BuildGun->GetCurrentState();
		UFGBuildGunStateBuild* BuildState = dynamic_cast<UFGBuildGunStateBuild*>(State);
		if(IsValid(BuildState)) {
			AFGHologram* Hologram = BuildState->GetHologram();
			AFINArrowModuleHolo* ArrowHolo = dynamic_cast<AFINArrowModuleHolo*>(Hologram);
			if(IsValid(ArrowHolo)) {
				ArrowHolo->Anchors = Module->Anchors;
				ArrowHolo->bNeedRebuild = true;
			}
		}
	}
}


AFINArrowModuleHolo::AFINArrowModuleHolo() {
	PopupClass = FSoftObjectPath(TEXT("/Game/FactoryGame/Interface/UI/InGame/Widget_Popup.Widget_Popup_C"));
	PopupContentClass = FSoftObjectPath(TEXT("/FicsItNetworks/UI/Misc/BPW_FIN_PanelTraceConfigPopup.BPW_FIN_PanelTraceConfigPopup_C"));
	
}

AActor* AFINArrowModuleHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID constructionID) {
	return Super::Construct(out_children, constructionID);
}

void AFINArrowModuleHolo::RebuildParts() {
	for (UStaticMeshComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();
	if (mBuildClass) {
		AFINArrowModuleBase* DefaultObject =	Cast<AFINArrowModuleBase>(mBuildClass->GetDefaultObject());
		DefaultObject->RebuildComponents(this, RootComponent, Anchors, Parts);
		DefaultObject->LastConfiguredAnchors = Anchors;
		DefaultObject->HasConfiguration = true;
		RootComponent->SetMobility(EComponentMobility::Movable);
		for (UStaticMeshComponent* Part : Parts) {
			Part->SetMobility(EComponentMobility::Movable);
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


void AFINArrowModuleHolo::ConstructParts() {
	if (mBuildClass) {
		AFINArrowModuleBase* DefaultObject =	Cast<AFINArrowModuleBase>(mBuildClass->GetDefaultObject());
		if(DefaultObject->HasConfiguration) {
			Anchors = DefaultObject->LastConfiguredAnchors;			
		}else {
			Anchors = DefaultObject->Anchors;
		}

		RebuildParts();
	}
}

void AFINArrowModuleHolo::CheckValidFloor() {
	Super::CheckValidFloor();
}

void AFINArrowModuleHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);
	if(auto Base = Cast<AFINArrowModuleBase>(inBuildable)) {
		Base->Anchors = Anchors;
	}
}

bool AFINArrowModuleHolo::IsValidHitResult(const FHitResult& hit) const {
	return Super::IsValidHitResult(hit);
}


void AFINArrowModuleHolo::SetHologramLocationAndRotation(const FHitResult& hit) {
	Super::SetHologramLocationAndRotation(hit);
}




void AFINArrowModuleHolo::BeginPlay() {
	Super::BeginPlay();
	
	ConstructParts();

	if(APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController())) {
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			Subsystem->AddMappingContext(MappingContext, 1000);
			TArray<FName> ActionNames;
			for (auto Mapping : MappingContext->GetMappings()) {
				ActionNames.Add(Mapping.GetMappingName());
			}
			OnInputMappingBound(ActionNames);
			auto var = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			AFGCharacterPlayer* AFG = Cast<AFGCharacterPlayer>(var);
			if(UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(AFG->GetBuildGun()->InputComponent)) {
				ConfigureAction->bConsumeInput = true;
				EIC->BindAction(ConfigureAction, ETriggerEvent::Triggered, this, &AFINArrowModuleHolo::ConfigureProperties);
			}
		}
	}
}

void AFINArrowModuleHolo::ConfigureProperties(const FInputActionValue& Value) {
	ShowPropertyDialog();
	
}

void AFINArrowModuleHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(bNeedRebuild || bNeedColorUpdate) {
		bNeedRebuild = false;
		bNeedColorUpdate = false;
		
		RebuildParts();
	}
}

void AFINArrowModuleHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	
	if(APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController())) {
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			Subsystem->RemoveMappingContext(MappingContext);
		}
	}
}

bool AFINArrowModuleHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	return Super::DoMultiStepPlacement(isInputFromARelease);
}

int32 AFINArrowModuleHolo::GetBaseCostMultiplier() const {
	return Super::GetBaseCostMultiplier();
}

void AFINArrowModuleHolo::PopupClosed(bool bConfirmed) {
	if(bConfirmed) {
		bNeedRebuild = true;
	}
}


void AFINArrowModuleHolo::ShowPropertyDialog() {
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
		UFINPanelTraceConfigPopup* PopupContent = CreateWidget<UFINPanelTraceConfigPopup>(GetWorld()->GetFirstPlayerController(), PopupContentClass.LoadSynchronous());
		PopupContent->Hologram = this;

		FPopupClosed PopupClosedDelegate;
		PopupClosedDelegate.BindDynamic(this, &AFINArrowModuleHolo::PopupClosed);
		UFGBlueprintFunctionLibrary::AddPopupWithContent(
			GetWorld()->GetFirstPlayerController(),
			FText::FromString(TEXT("FicsIt-Networks Panel Trace Configuration")),
			FText(),
			PopupClosedDelegate,
			PopupContent,
			PID_OK_CANCEL,
			this);
	});
}

TSharedRef<SWidget> UFINPanelTraceConfigPopup::RebuildWidget() {
	TraceOuter.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("None", FINPanelTraceEnd_None)));
	TraceOuter.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Straight", FINPanelTraceEnd_Straight)));
	TraceOuter.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Arrow", FINPanelTraceEnd_ArrowOut)));
	TraceOuter.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Blocked", FINPanelTraceEnd_Blockage)));
	TraceOuter.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Short Blocked", FINPanelTraceEnd_RecessedBlockage)));

	TraceInner.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Cap Square", FINPanelTraceStart_CapSquare)));
	TraceInner.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Cap Rounded", FINPanelTraceStart_CapRound)));
	TraceInner.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Cap Triangle", FINPanelTraceStart_Miter)));
	TraceInner.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("None", FINPanelTraceStart_None)));
	TraceInner.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Midpoint", FINPanelTraceStart_Half)));

	Center.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("None", FIN_PanelArrowCrossing_Lines)));
	Center.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Circle", FIN_PanelArrowCrossing_Dot)));
	Center.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Horizontal Bridge", FIN_PanelArrowCrossing_BridgeH)));
	Center.Add(FFINArrowOptionType(MakeShared<FFINIconTextIntegerOption>("Vertical Bridge", FIN_PanelArrowCrossing_BridgeV)));
	return SNew(SBox)
	.MinDesiredWidth(FOptionalSize(200))
	.MinDesiredHeight(FOptionalSize(300))
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SUniformGridPanel)
			+SUniformGridPanel::Slot(0,0)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboUpArrow_Outer, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceOuter)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceOuter(0, static_cast<EFINPanelTraceEndTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceOuterItem(0, TraceOuter))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboUpArrow_Outer->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(0, 1)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboUpArrow_Inner, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceInner)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceInner(0, static_cast<EFINPanelTraceStartTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceInnerItem(0, TraceInner))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboUpArrow_Inner->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(0,2)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TxtUpArrowColor, SEditableTextBox)
					.Text(GetTraceColorText(0))
					.IsEnabled(!IsTraceColorInherit(0))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) {
						SetTraceColorFromText(0, TxtUpArrowColor->GetText());
					})
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(IsTraceColorInherit(0))
					.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState) {
						SetTraceColorInherit(0, CheckBoxState == ECheckBoxState::Checked, TxtUpArrowColor);
					})
				]
			]
			+SUniformGridPanel::Slot(0,3)
			[
				SNew(SSpacer)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SAssignNew(CboCenterStyle, SComboBox<FFINArrowOptionType>)
			.OnGenerateWidget_Static(CreateItem)
			.OptionsSource(&Center)
			.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
				SetCenter( static_cast<EFINPanelArrowCrossingTypes>(Value.Get()->Value));
			})
			.InitiallySelectedItem(GetCenterItem(Center))
			[
				SNew(STextBlock).Text_Lambda([this] {
					FFINArrowOptionType FfinIconTextIntegerOption = CboCenterStyle->GetSelectedItem();
					if (FfinIconTextIntegerOption.IsValid()){
						return FText::FromString(FfinIconTextIntegerOption->Text);
					}
					return FText::FromString(TEXT("Invalid Option"));
				})
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SAssignNew(TxtAnchorColor, SEditableTextBox)
			.Text(GetCenterColor())
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) {
				SetCenterColorText(TxtAnchorColor->GetText());
			})
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SUniformGridPanel)
			+SUniformGridPanel::Slot(0,0)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboLeftArrow_Outer, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceOuter)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceOuter(3, static_cast<EFINPanelTraceEndTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceOuterItem(3, TraceOuter))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboLeftArrow_Outer->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(1,0)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboLeftArrow_Inner, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceInner)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceInner(3, static_cast<EFINPanelTraceStartTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceInnerItem(3, TraceInner))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboLeftArrow_Inner->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(0,1)
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TxtLeftArrowColor, SEditableTextBox)
					.Text(GetTraceColorText(3))
					.IsEnabled(!IsTraceColorInherit(3))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) {
						SetTraceColorFromText(3, TxtLeftArrowColor->GetText());
					})
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(IsTraceColorInherit(3))
					.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState) {
						SetTraceColorInherit(3, CheckBoxState == ECheckBoxState::Checked, TxtLeftArrowColor);
					})
				]
			]
			+SUniformGridPanel::Slot(1,1)
			[
				SNew(SSpacer)
			]
			+SUniformGridPanel::Slot(2,0)
			[
				SNew(SSpacer)
			]
			+SUniformGridPanel::Slot(2,1)
			[
				SNew(SSpacer)
			]
			+SUniformGridPanel::Slot(3,1)
			[
				SNew(SSpacer)
			]
			+SUniformGridPanel::Slot(3,0)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboRightArrow_Inner, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceInner)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceInner(1, static_cast<EFINPanelTraceStartTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceInnerItem(1, TraceInner))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboRightArrow_Inner->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(4,0)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboRightArrow_Outer, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceOuter)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceOuter(1, static_cast<EFINPanelTraceEndTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceOuterItem(1, TraceOuter))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboRightArrow_Outer->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(4,1)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TxtRightArrowColor, SEditableTextBox)
					.Text(GetTraceColorText(1))
					.IsEnabled(!IsTraceColorInherit(1))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) {
						SetTraceColorFromText(1, TxtRightArrowColor->GetText());
					})
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(IsTraceColorInherit(1))
					.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState) {
						SetTraceColorInherit(1, CheckBoxState == ECheckBoxState::Checked, TxtRightArrowColor);
					})
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SUniformGridPanel)
			+SUniformGridPanel::Slot(0,0)
			[
				SNew(SSpacer)
			]
			+SUniformGridPanel::Slot(0,1)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TxtDownArrowColor, SEditableTextBox)
					.Text(GetTraceColorText(2))
					.IsEnabled(!IsTraceColorInherit(2))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) {
						SetTraceColorFromText(2, TxtDownArrowColor->GetText());
					})
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(IsTraceColorInherit(2))
					.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState) {
						SetTraceColorInherit(2, CheckBoxState == ECheckBoxState::Checked, TxtDownArrowColor);
					})
				]
			]
			+SUniformGridPanel::Slot(0,2)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboDownArrow_Inner, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceInner)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceInner(2, static_cast<EFINPanelTraceStartTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceInnerItem(2, TraceInner))
				[
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboDownArrow_Inner->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
			+SUniformGridPanel::Slot(0,3)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(CboDownArrow_Outer, SComboBox<FFINArrowOptionType>)
				.OnGenerateWidget_Static(CreateItem)
				.OptionsSource(&TraceOuter)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FFINIconTextIntegerOption>& Value, ESelectInfo::Type) {
					SetTraceOuter(2, static_cast<EFINPanelTraceEndTypes>(Value.Get()->Value));
				})
				.InitiallySelectedItem(GetTraceOuterItem(2, TraceOuter))
				[
					//CreateItem(CboDownArrow_Outer->GetSelectedItem())
					SNew(STextBlock).Text_Lambda([this] {
						FFINArrowOptionType FfinIconTextIntegerOption = CboDownArrow_Outer->GetSelectedItem();
						if (FfinIconTextIntegerOption.IsValid()){
							return FText::FromString(FfinIconTextIntegerOption->Text);
						}
						return FText::FromString(TEXT("Invalid Option"));
					})
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSpacer)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.OnClicked_Lambda([this] {
				UFGBlueprintFunctionLibrary::ClosePopup(GetWorld()->GetFirstPlayerController());
				return FReply::Handled();
			})
			.HAlign(HAlign_Center)
			.Text(FText::FromString("Close"))
		]
	];
}

void UFINPanelTraceConfigPopup::SetTraceOuter(int Index, EFINPanelTraceEndTypes Type) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			Hologram->Anchors[0].Arrows[Index].OuterEnd = Type;
			Hologram->bNeedRebuild = true;
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}


void UFINPanelTraceConfigPopup::SetTraceInner(int Index, EFINPanelTraceStartTypes Type) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			Hologram->Anchors[0].Arrows[Index].InnerEnd = Type;
			Hologram->bNeedRebuild = true;
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}

void UFINPanelTraceConfigPopup::SetCenter(EFINPanelArrowCrossingTypes Type) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		Hologram->Anchors[0].Type = Type;
		Hologram->bNeedRebuild = true;
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}
void UFINPanelTraceConfigPopup::SetCenterColorText(FText ColorIn) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		Hologram->Anchors[0].AnchorColor = UMCPBlueprintLibrary::HexStringToLinearColor(ColorIn.ToString());
		Hologram->bNeedColorUpdate = true;
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}

FFINArrowOptionType UFINPanelTraceConfigPopup::GetTraceOuterItem(int Index, TArray<FFINArrowOptionType>& Source) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			int V = Hologram->Anchors[0].Arrows[Index].OuterEnd;
			for (auto Item : Source) {
				if(Item.Get()->Value == V) {
					return Item;
				}
			}
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
	return Source[0];
}

FFINArrowOptionType UFINPanelTraceConfigPopup::GetTraceInnerItem(int Index, TArray<FFINArrowOptionType>& Source) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			int V = Hologram->Anchors[0].Arrows[Index].InnerEnd;
			for (auto Item : Source) {
				if(Item.Get()->Value == V) {
					return Item;
				}
			}
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
	return Source[0];
}

FFINArrowOptionType UFINPanelTraceConfigPopup::GetCenterItem(TArray<FFINArrowOptionType>& Source) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		int V = Hologram->Anchors[0].Type;
		for (auto Item : Source) {
			if(Item.Get()->Value == V) {
				return Item;
			}
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
	return Source[0];
}

void UFINPanelTraceConfigPopup::SetTraceColorFromText(int Index, const FText& Text) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			Hologram->Anchors[0].Arrows[Index].ArrowColor = UMCPBlueprintLibrary::HexStringToLinearColor(Text.ToString());
			Hologram->bNeedColorUpdate = true;
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}

void UFINPanelTraceConfigPopup::SetTraceColorInherit(int Index, bool Value, TSharedPtr<SEditableTextBox> BoundTextField) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			Hologram->Anchors[0].Arrows[Index].InheritColor = Value;
			BoundTextField->SetEnabled(!Value);
			Hologram->bNeedColorUpdate = true;
		}else {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
		}
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
}

bool UFINPanelTraceConfigPopup::IsTraceColorInherit(int Index) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			return Hologram->Anchors[0].Arrows[Index].InheritColor;
		}
		UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
	return true;
}

FText UFINPanelTraceConfigPopup::GetTraceColorText(int Index) {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		if(Hologram->Anchors[0].Arrows.Num() > Index) {
			const FLinearColor V = Hologram->Anchors[0].Arrows[Index].ArrowColor;
			return FText::FromString(UMCPBlueprintLibrary::ColorToHexString(V));
		}
		UE_LOG(LogFicsItNetworks, Error, TEXT("Arrow is missing"));
	}else {
		UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	}
	return FText::FromString(TEXT("#000000"));
}


TSharedRef<SWidget> UFINPanelTraceConfigPopup::CreateItem(FFINArrowOptionType Value) {
	auto V = Value.Get();
	if(IsValid(V->Icon)) {
		TSharedPtr<FSlateImageBrush> ImageBrush = MakeShared<FSlateImageBrush>(V->Icon, FVector2D(20, 20));
		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()[
				SNew(SImage).Image(ImageBrush.Get())
			]
			+SHorizontalBox::Slot()[
				SNew(STextBlock).Text(FText::FromString(V->Text))
			]
		;
	}
	return SNew(STextBlock).Text(FText::FromString(V->Text));
}

FText UFINPanelTraceConfigPopup::GetCenterColor() {
	if(IsValid(Hologram) && !Hologram->Anchors.IsEmpty()) {
		FLinearColor V = Hologram->Anchors[0].AnchorColor;
		return FText::FromString(UMCPBlueprintLibrary::ColorToHexString(V));
	}
	UE_LOG(LogFicsItNetworks, Error, TEXT("Anchors is empty"));
	return FText::FromString(TEXT("#000000"));
}

void UFINPanelTraceConfigPopup::NativeConstruct() {
	Super::NativeConstruct();
	
	UOverlaySlot* OSlot = Cast<UOverlaySlot>(Slot->Parent->Slot);
	OSlot->SetPadding(FMargin(0));
	OSlot->SetHorizontalAlignment(HAlign_Fill);
	OSlot->SetVerticalAlignment(VAlign_Fill);

}
