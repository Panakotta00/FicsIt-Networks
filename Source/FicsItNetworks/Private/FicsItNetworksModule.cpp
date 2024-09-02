#include "FicsItNetworksModule.h"

#include "UI/FINCopyUUIDButton.h"
#include "Computer/FINComputerRCO.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "FicsItKernel/FicsItFS/Library/Tests.h"
#include "AssetRegistryModule.h"
#include "FGCharacterPlayer.h"
#include "FGGameMode.h"
#include "FGGameState.h"
#include "FicsItReflection.h"
#include "Components/VerticalBox.h"
#include "Computer/FINComputerSubsystem.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"
#include "UI/FINEditLabel.h"
#include "UI/FINStyle.h"
#include "UObject/CoreRedirects.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworks);
DEFINE_LOG_CATEGORY(LogFicsItNetworksNet);

IMPLEMENT_GAME_MODULE(FFicsItNetworksModule, FicsItNetworks);

FDateTime FFicsItNetworksModule::GameStart;

void AFGBuildable_Dismantle_Implementation(CallScope<void(*)(IFGDismantleInterface*)>& scope, IFGDismantleInterface* self_r) {
	AFGBuildable* self = dynamic_cast<AFGBuildable*>(self_r);
	TInlineComponentArray<UFINModuleSystemPanel*> panels;
	self->GetComponents(panels);
	for (UFINModuleSystemPanel* panel : panels) {
		TArray<AActor*> modules;
		panel->GetModules(modules);
		for (AActor* module : modules) {
			module->Destroy();
		}
	}
}

void AFGBuildable_GetDismantleRefund_Implementation(CallScope<void(*)(const IFGDismantleInterface*, TArray<FInventoryStack>&, bool)>& scope, const IFGDismantleInterface* self_r, TArray<FInventoryStack>& refund, bool noCost) {
	const AFGBuildable* self = dynamic_cast<const AFGBuildable*>(self_r);
	TInlineComponentArray<UFINModuleSystemPanel*> panels;
	self->GetComponents(panels);
	for (UFINModuleSystemPanel* panel : panels) {
		panel->GetDismantleRefund(refund, noCost);
	}
}

struct ClassChange {
	FString From;
	FString To;
	TArray<ClassChange> Children;
};

void AddRedirects(FString FromParent, FString ToParent, const ClassChange& Change, TArray<FCoreRedirect>& Redirects) {
	FromParent += "/" + Change.From;
	ToParent += "/" + Change.To;
	if (Change.Children.Num() < 1) {
		FString From = FString::Printf(TEXT("%s.%s_C"), *FromParent, *Change.From);
		FString To = FString::Printf(TEXT("%s.%s_C"), *ToParent, *Change.To);
		UE_LOG(LogFicsItNetworks, Warning, TEXT("From: '%s' To: '%s'"), *From, *To);
		Redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, From, To});
	}
	for (const ClassChange& Child : Change.Children) {
		AddRedirects(FromParent, ToParent, Child, Redirects);
	}
}

void InventorSlot_CreateWidgetSlider_Hook(FBlueprintHookHelper& HookHelper) {
	UUserWidget* self = Cast<UUserWidget>(HookHelper.GetContext());
	UObject* InventorySlot = HookHelper.GetContext();
	TObjectPtr<UObject>* WidgetPtr = HookHelper.GetOutVariablePtr<FObjectProperty>();
	UUserWidget* Widget = Cast<UUserWidget>(WidgetPtr->Get());
	UVerticalBox* MenuList = Cast<UVerticalBox>(Widget->GetWidgetFromName("VerticalBox_0"));

	if (IsValid(UFINCopyUUIDButton::GetFileSystemStateFromSlotWidget(self))) {
		UFINCopyUUIDButton* UUIDButton = NewObject<UFINCopyUUIDButton>(MenuList->GetOuter());
		UUIDButton->InitSlotWidget(self);
		MenuList->AddChildToVerticalBox(UUIDButton);
	}

	if (IsValid(UFINEditLabel::GetLabelContainerFromSlot(self).GetObject())) {
		UFINEditLabel* EditLabel = NewObject<UFINEditLabel>(MenuList->GetOuter());
		EditLabel->InitSlotWidget(self);
		MenuList->AddChildToVerticalBox(EditLabel);
	}
}
UE_DISABLE_OPTIMIZATION_SHIP
void FFicsItNetworksModule::StartupModule(){
	FSlateStyleRegistry::UnRegisterSlateStyle(FFINStyle::GetStyleSetName());
	FFINStyle::Initialize();

	CodersFileSystem::Tests::TestPath();
	
	GameStart = FDateTime::Now();
	
	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINNetworkConnector"), TEXT("/Script/FicsItNetworks.FINAdvancedNetworkConnectionComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Splitter/Splitter.Splitter_C"), TEXT("/FicsItNetworks/Components/CodeableSplitter/CodeableSplitter.CodeableSplitter_C")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Merger/Merger.Merger_C"), TEXT("/FicsItNetworks/Components/CodeableMerger/CodeableMerger.CodeableMerger_C")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStateEEPROMLua"), TEXT("/Script/FicsItNetworksLua.FINStateEEPROMLua")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINComputerProcessorLua"), TEXT("/Script/FicsItNetworksLua.FINComputerProcessorLua")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLuaProcessor"), TEXT("/Script/FicsItNetworksLua.FINLuaProcessor")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLuaProcessorStateStorage"), TEXT("/Script/FicsItNetworksLua.FINLuaProcessorStateStorage")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.EFINMetaRuntimeState"), TEXT("/Script/FicsItNetworksLua.EFINReflectionMetaRuntimeState")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintPropertyMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionPropertyMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintFunctionMetaParameter"), TEXT("/Script/FicsItNetworksLua.FFINReflectionFunctionParameterMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintFunctionMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionFunctionMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintSignalMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionSignalMeta")});
	
	AddRedirects(TEXT(""), TEXT(""),
		{	TEXT("FicsItNetworks/Components/RozeModularSystem"), TEXT("FicsItNetworks/Components/MicroControlPanels"),{
			{TEXT("Enclosures"), TEXT("MicroControlPanels"), {
				{TEXT("1pointReceiver"), TEXT("MCP_1Point"), {
					{TEXT("Item_1pointbox"), TEXT("MCP_1Point")},
					{TEXT("Item_1pointCenterBox"), TEXT("MCP_1Point_Center")},
				}},
				{TEXT("2pointReceiver"), TEXT("MCP_2Point"), {
					{TEXT("Item_2pointbox"), TEXT("MCP_2Point")},
				}},
				{TEXT("3pointReceiver"), TEXT("MCP_3Point"), {
					{TEXT("Item_3pointbox"), TEXT("MCP_3Point")},
				}},
				{TEXT("6pointReceiver"), TEXT("MCP_6Point"), {
					{TEXT("Item_6pointbox"), TEXT("MCP_6Point")},
				}},
			}},
			{TEXT("Modules"), TEXT("Modules"), {
				{TEXT("2positionswitch"), TEXT("2PosSwitch"), {
					{TEXT("2PositionSwitch-Item"), TEXT("MCP_Mod_2Pos_Switch")}
				}},
				{TEXT("Indicator"), TEXT("Indicator"), {
					{TEXT("Item_IndicatorModule"), TEXT("MCP_Mod_Indicator")}
				}},
				{TEXT("MushroomPushbutton"), TEXT("MushroomPushbutton"), {
					{TEXT("Item_MushroomPushButtonModule"), TEXT("MCP_Mod_MushroomPushButtonModule")}
				}},
				{TEXT("Plug"), TEXT("Plug"), {
					{TEXT("Item_PlugModule"), TEXT("MCP_Mod_Plug")}
				}},
				{TEXT("PushButton"), TEXT("PushButton"), {
					{TEXT("PushButtonModule-Item"), TEXT("MCP_Mod_PushButton")}
				}}
			}}
		},
	}, redirects);
		
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/FicsItNetworks"));
	AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);
	
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByPath(TEXT("/FicsItNetworks"), AssetData, true);

	for (const FAssetData& Asset : AssetData) {
		FString NewPath = Asset.GetObjectPathString();
		FString OldPath = TEXT("/Game") + Asset.GetObjectPathString();
		if (Asset.AssetClass == TEXT("Blueprint") || Asset.AssetClass == TEXT("WidgetBlueprint")) { // TODO: Check if AssetClassPath works, and how?
			NewPath += TEXT("_C");
			OldPath += TEXT("_C");
		}
		// UE_LOG(LogFicsItNetworks, Warning, TEXT("FIN Redirect: '%s' -> '%s'"), *OldPath, *NewPath);
		redirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_AllMask, OldPath, NewPath));
	}

	FCoreRedirects::AddRedirectList(redirects, "FIN-Code");
	
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::Dismantle_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_Dismantle_Implementation);
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::GetDismantleRefund_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_GetDismantleRefund_Implementation);
		
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::BeginPlay, (void*)GetDefault<AFGCharacterPlayer>(), [](AActor* self) {
			AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
			if (character) {
				AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
				if (SubSys) SubSys->AttachWidgetInteractionToPlayer(character);
			}
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::EndPlay, (void*)GetDefault<AFGCharacterPlayer>(), [](AActor* self, EEndPlayReason::Type Reason) {
			AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
			if (character) {
				AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
				if (SubSys) SubSys->DetachWidgetInteractionToPlayer(character);
			}
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameMode::PostLogin, (void*)GetDefault<AFGGameMode>(), [](AFGGameMode* gm, APlayerController* pc) {
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
				gm->RegisterRemoteCallObjectClass(UFINComputerRCO::StaticClass());

				UClass* ModuleRCO = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Components/ModularPanel/Modules/Module_RCO.Module_RCO_C"));
				check(ModuleRCO);
				gm->RegisterRemoteCallObjectClass(ModuleRCO);
			}
		});

		// Copy FS UUID Item Context Menu Entry //
		UClass* Slot = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Interface/UI/InGame/InventorySlots/Widget_InventorySlot.Widget_InventorySlot_C"));
		check(Slot);
		UFunction* Function = Slot->FindFunctionByName(TEXT("CreateSplitSlider"));
		UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
		HookManager->HookBlueprintFunction(Function, InventorSlot_CreateWidgetSlider_Hook, EPredefinedHookOffset::Return);

#endif
	});
}
UE_ENABLE_OPTIMIZATION_SHIP

void FFicsItNetworksModule::ShutdownModule() {
	FFINStyle::Shutdown();
}

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
