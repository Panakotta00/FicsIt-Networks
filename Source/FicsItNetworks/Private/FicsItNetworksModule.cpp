#include "FicsItNetworksModule.h"

#include "UI/FINCopyUUIDButton.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "FGCharacterPlayer.h"
#include "FGCheatManager.h"
#include "FGGameMode.h"
#include "FGGameRulesSubsystem.h"
#include "FGGameState.h"
#include "FicsItNetworksMisc.h"
#include "FicsItReflection.h"
#include "FINArrowModuleHolo.h"
#include "FINDependencies.h"
#include "ReflectionHelper.h"
#include "SubsystemActorManager.h"
#include "TextBlock.h"
#include "VerticalBoxSlot.h"
#include "WrapBox.h"
#include "WrapBoxSlot.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/VerticalBox.h"
#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/NativeHookManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "UI/FINEditLabel.h"
#include "UI/FINStyle.h"
#include "UObject/CoreRedirects.h"

IMPLEMENT_GAME_MODULE(FFicsItNetworksModule, FicsItNetworks);

DEFINE_LOG_CATEGORY(LogFicsItNetworks);

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
	TObjectPtr<UObject>* WidgetPtr = HookHelper.GetOutVariableHelper()->GetVariablePtr<FObjectProperty>(TEXT("ReturnValue"));
	UUserWidget* Widget = Cast<UUserWidget>(WidgetPtr->Get());
	UVerticalBox* MenuList = Cast<UVerticalBox>(Widget->GetWidgetFromName("VerticalBox_0"));

	if (UFINCopyUUIDButton::GetFileSystemStateFromSlotWidget(self).IsValid()) {
		UFINCopyUUIDButton* UUIDButton = NewObject<UFINCopyUUIDButton>(MenuList->GetOuter());
		UUIDButton->InitSlotWidget(self);
		MenuList->AddChildToVerticalBox(UUIDButton);
	}

	if (UFINEditLabel::GetLabelFromSlot(self).IsSet()) {
		UFINEditLabel* EditLabel = NewObject<UFINEditLabel>(MenuList->GetOuter());
		EditLabel->InitSlotWidget(self);
		MenuList->AddChildToVerticalBox(EditLabel);
	}
}

void ResearchNodeInfoWidget_CanResearch_Hook(FBlueprintHookHelper& HookHelper) {
	UObject* Widget = HookHelper.GetContext();
	bool bNoUnlockCost = AFGGameRulesSubsystem::Get(Widget)->GetNoUnlockCost();

	TSubclassOf<UFGSchematic> schematic = FReflectionHelper::GetObjectPropertyValue<UClass>(Widget, TEXT("mSchematic"));

	bool& canResearch = *HookHelper.GetOutVariableHelper()->GetVariablePtr<TProperty<bool, FBoolProperty>>(TEXT("Can Research"));
	canResearch = canResearch && (bNoUnlockCost || UFGSchematic::AreSchematicDependenciesMet(schematic, Widget));
}
UE_DISABLE_OPTIMIZATION_SHIP
void ResearchNodeInfoWidget_UpdateState_Hook(FBlueprintHookHelper& HookHelper) {
	UUserWidget* Widget = Cast<UUserWidget>(HookHelper.GetContext());
	TSubclassOf<UFGSchematic> schematic = FReflectionHelper::GetObjectPropertyValue<UClass>(Widget, TEXT("mSchematic"));

	UWrapBox* wrapBox = FReflectionHelper::GetObjectPropertyValue<UWrapBox>(Widget, TEXT("mCostSlotsContainer"));
	TArray<UFGAvailabilityDependency*> dependencies;
	UFGSchematic::GetSchematicDependencies(schematic, dependencies);
	TSubclassOf<UUserWidget> dependencyWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/FicsItNetworks/UI/Misc/BPW_FIN_Dependency.BPW_FIN_Dependency_C"));
	for (UFGAvailabilityDependency* dependency : dependencies) {
		UFINDependency* FINDependency = Cast<UFINDependency>(dependency);
		if (!IsValid(FINDependency)) continue;
		UUserWidget* dependencyWidget = Widget->WidgetTree->ConstructWidget<UUserWidget>(dependencyWidgetClass);
		FReflectionHelper::SetPropertyValue<FObjectProperty>(dependencyWidget, TEXT("Dependency"), FINDependency);
		UWrapBoxSlot* slot = wrapBox->AddChildToWrapBox(dependencyWidget);
		slot->SetFillEmptySpace(true);
	}
}
UE_ENABLE_OPTIMIZATION_SHIP
#define FIN_CoreRedirect(Type, OldName, NewName) \
	redirects.Add(FCoreRedirect{ECoreRedirectFlags:: Type, \
		TEXT(OldName), \
		TEXT(NewName)})

UE_DISABLE_OPTIMIZATION_SHIP
void FFicsItNetworksModule::StartupModule(){
	FSlateStyleRegistry::UnRegisterSlateStyle(FFINStyle::GetStyleSetName());
	FFINStyle::Initialize();

	TArray<FCoreRedirect> redirects;

	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINNetworkConnector"), TEXT("/Script/FicsItNetworks.FINAdvancedNetworkConnectionComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Splitter/Splitter.Splitter_C"), TEXT("/FicsItNetworks/Components/CodeableSplitter/Build_CodeableSplitter.Build_CodeableSplitter_C")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Merger/Merger.Merger_C"), TEXT("/FicsItNetworks/Components/CodeableMerger/Build_CodeableMerger.Build_CodeableMerger_C")});

	// Begin v0.3.21
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/CodeableMerger/CodeableMerger.CodeableMerger_C",
		"/FicsItNetworks/Buildings/Components/CodeableMerger/Build_CodeableMerger.Build_CodeableMerger_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/CodeableSplitter/CodeableSplitter.CodeableSplitter_C",
		"/FicsItNetworks/Buildings/Components/CodeableSplitter/Build_CodeableSplitter.Build_CodeableSplitter_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/IndicatorPole/IndicatorPole.IndicatorPole_C",
		"/FicsItNetworks/Buildings/Components/IndicatorPole/Build_IndicatorPole.Build_IndicatorPole_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/VehicleScanner/VehicleScanner.VehicleScanner_C",
		"/FicsItNetworks/Buildings/Components/VehicleScanner/Build_VehicleScanner.Build_VehicleScanner_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/WirelessAccessPoint/WirelessAccessPoint.WirelessAccessPoint_C",
		"/FicsItNetworks/Buildings/Network/WirelessAccessPoint/Build_WirelessAccessPoint.Build_WirelessAccessPoint_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/Speakers/Speakers.Speakers_C",
		"/FicsItNetworks/Buildings/Components/Speakers/Build_Speakers.Build_Speakers_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/NetworkRouter/NetworkRouter.NetworkRouter_C",
		"/FicsItNetworks/Buildings/Network/NetworkRouter/Build_NetworkRouter.Build_NetworkRouter_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/Screen/Build_Screen.Build_Screen_C",
		"/FicsItNetworks/Buildings/Components/Screen/Build_Screen.Build_Screen_C");
	FIN_CoreRedirect(Type_Class,
        "/FicsItNetworks/Components/ModularIndicatorPole/Build_ModularIndicatorPole.Build_ModularIndicatorPole_C",
        "/FicsItNetworks/Buildings/Components/ModularIndicatorPole/Build_ModularIndicatorPole.Build_ModularIndicatorPole_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularIndicatorPole/IndicatorModules/Build_PoleIndicatorModule.Build_PoleIndicatorModule_C",
		"/FicsItNetworks/Buildings/Components/ModularIndicatorPole/IndicatorModules/Build_PoleIndicatorModule.Build_PoleIndicatorModule_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/LargeControlPanel/LargeControlPanel.LargeControlPanel_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/LargeControlPanel/Build_LargeControlPanel.Build_LargeControlPanel_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/LargeVerticalControlPanel/LargeVerticalControlPanel.LargeVerticalControlPanel_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/LargeVerticalControlPanel/Build_LargeVerticalControlPanel.Build_LargeVerticalControlPanel_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModuleButton.ModuleButton_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/SimpleButton/Build_ModuleButton.Build_ModuleButton_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModuleSwitch.ModuleSwitch_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Switch/Build_ModuleSwitch.Build_ModuleSwitch_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModulePotentiometer.ModulePotentiometer_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/SimplePotentiometer/Build_ModulePotentiometer.Build_ModulePotentiometer_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModuleScreen.ModuleScreen_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Screen/Build_ModuleScreen.Build_ModuleScreen_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModuleTextDisplay.ModuleTextDisplay_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/TextDisplay/Build_ModuleTextDisplay.Build_ModuleTextDisplay_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/ModuleStopButton.ModuleStopButton_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/StopButton/Build_ModuleStopButton.Build_ModuleStopButton_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/BigGuage/Module_BigGuage.Module_BigGuage_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/BigGauge/Build_ModuleBigGauge.Build_ModuleBigGauge_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/Label/Module_Label_1x1.Module_Label_1x1_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Label/Build_ModuleLabel_1x1.Build_ModuleLabel_1x1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/ModularPanel/Modules/Label/Module_Label_2x1.Module_Label_2x1_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Label/Build_ModuleLabel_2x1.Build_ModuleLabel_2x1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/PushButton/MCP_Mod_PushButton.MCP_Mod_PushButton_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/PushButton/Build_ModulesPushButton.Build_ModulesPushButton_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Plug/MCP_Mod_Plug.MCP_Mod_Plug_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Plug/Build_ModulePlug.Build_ModulePlug_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/2PosSwitch/MCP_Mod_2Pos_Switch.MCP_Mod_2Pos_Switch_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/2PosSwitch/Build_Module2PosSwitch.Build_Module2PosSwitch_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/3PosSwitch/MCP_Mod_3Pos_Switch.MCP_Mod_3Pos_Switch_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/3PosSwitch/Build_Module3PosSwitch.Build_Module3PosSwitch_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Buzzer/MCP_Mod_Buzzer.MCP_Mod_Buzzer_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Buzzer/Build_ModuleBuzzer.Build_ModuleBuzzer_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Encoder/MCP_Mod_Encoder.MCP_Mod_Encoder_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Encoder/Build_ModuleEncoder.Build_ModuleEncoder_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Guage/MCP_Mod_Guage.MCP_Mod_Guage_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Gauge/Build_ModuleGauge.Build_ModuleGauge_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/MushroomPushbutton/MCP_Mod_MushroomPushButtonModule.MCP_Mod_MushroomPushButtonModule_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/MushroomPushButton/Build_ModuleMushroomPushButton.Build_ModuleMushroomPushButton_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Indicator/MCP_Mod_Indicator.MCP_Mod_Indicator_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Indicator/Build_ModuleIndicator.Build_ModuleIndicator_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/MicroDisplay/MCP_Mod_LargeMicroDIsplay.MCP_Mod_LargeMicroDIsplay_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/MicroDisplay/Build_ModuleMicroDisplay_Large.Build_ModuleMicroDisplay_Large_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/MicroDisplay/MCP_Mod_MicroDisplay.MCP_Mod_MicroDisplay_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/MicroDisplay/Build_ModuleMicroDisplay.Build_ModuleMicroDisplay_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/MicroDisplay/MCP_Mod_MicroDisplaySquare.MCP_Mod_MicroDisplaySquare_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/MicroDisplay/Build_ModuleMicroDisplay_Square.Build_ModuleMicroDisplay_Square_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Potentiometer/MCP_Mod_Potentiometer.MCP_Mod_Potentiometer_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Potentiometer/Build_ModulePotentiometer.Build_ModulePotentiometer_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/Modules/Potentiometer/MCP_Mod_PotWNum.MCP_Mod_PotWNum_C",
		"/FicsItNetworks/Buildings/ModularPanel/Modules/Potentiometer/Build_ModulePotentiometer_WithNumber.Build_ModulePotentiometer_WithNumber_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_1Point/MCP_1Point.MCP_1Point_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/MicroPanel1/Build_MicroPanel1.Build_MicroPanel1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_1Point/MCP_1Point_Center.MCP_1Point_Center_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/MicroPanel1/Build_MicroPanel1_Center.Build_MicroPanel1_Center_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_2Point/MCP_2Point.MCP_2Point_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/MicroPanel2/Build_MicroPanel2.Build_MicroPanel2_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_3Point/MCP_3Point.MCP_3Point_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/MicroPanel3/Build_MicroPanel3.Build_MicroPanel3_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_6Point/MCP_6Point.MCP_6Point_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/MicroPanel6/Build_MicroPanel6.Build_MicroPanel6_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/SizeablePanel/Build_SizeablePanel.Build_SizeablePanel_C",
		"/FicsItNetworks/Buildings/ModularPanel/Panels/SizeablePanel/Build_SizeablePanel.Build_SizeablePanel_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_LabelEditorTool/Equip_MCPLabelingTool.Equip_MCPLabelingTool_C",
		"/FicsItNetworks/Equipment/LabelingTool/Equip_LabelingTool.Equip_LabelingTool_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/MCP_LabelEditorTool/ED_MCPLabelEditor.ED_MCPLabelEditor_C",
		"/FicsItNetworks/Equipment/LabelingTool/Desc_LabelingTool.Desc_LabelingTool_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkCable/NetworkCable.NetworkCable_C",
		"/FicsItNetworks/Buildings/Network/NetworkCable/Build_NetworkCable.Build_NetworkCable_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkPole/NetworkPole.NetworkPole_C",
		"/FicsItNetworks/Buildings/Network/NetworkPole/Build_NetworkPole.Build_NetworkPole_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkWallPlug/NetworkWallPlug.NetworkWallPlug_C",
		"/FicsItNetworks/Buildings/Network/NetworkWallPlug/Build_NetworkWallPlug.Build_NetworkWallPlug_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/ThinNetworkCable/ThinNetworkCable.ThinNetworkCable_C",
		"/FicsItNetworks/Buildings/Network/ThinNetworkCable/Build_ThinNetworkCable.Build_ThinNetworkCable_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/SmallNetworkWallPlug/Item_SmallNetworkWallPlug.Item_SmallNetworkWallPlug_C",
		"/FicsItNetworks/Buildings/Network/SmallNetworkWallPlug/Build_SmallNetworkWallPlug.Build_SmallNetworkWallPlug_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkWallPlug_Double/NetworkWallPlug_Double.NetworkWallPlug_Double_C",
		"/FicsItNetworks/Buildings/Network/NetworkWallPlug_Double/Build_NetworkWallPlug_Double.Build_NetworkWallPlug_Double_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkAdapter/BP_NetworkAdapter.BP_NetworkAdapter_C",
		"/FicsItNetworks/Buildings/Network/NetworkAdapter/Build_NetworkAdapter.Build_NetworkAdapter_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkManager/Equip_NetworkManager.Equip_NetworkManager_C",
		"/FicsItNetworks/Equipment/NetworkManager/Equip_NetworkManager.Equip_NetworkManager_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Network/NetworkManager/ED_NetworkMng.ED_NetworkMng_C",
		"/FicsItNetworks/Equipment/NetworkManager/Desc_NetworkManager.Desc_NetworkManager_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/AdvancedNetworking.AdvancedNetworking_C",
		"/FicsItNetworks/Schematics/Research_AdvancedNetworking.Research_AdvancedNetworking_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/BasicComputers.BasicComputers_C",
		"/FicsItNetworks/Schematics/Research_BasicComputers.Research_BasicComputers_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/BasicNetworks.BasicNetworks_C",
		"/FicsItNetworks/Schematics/Research_BasicNetworks.Research_BasicNetworks_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/ComputerGraphics.ComputerGraphics_C",
		"/FicsItNetworks/Schematics/Research_ComputerGraphics.Research_ComputerGraphics_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/IOControll.IOControll_C",
		"/FicsItNetworks/Schematics/Research_IOControl.Research_IOControl_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/MicroControlPanel.MicroControlPanel_C",
		"/FicsItNetworks/Schematics/Research_MicroControlPanel.Research_MicroControlPanel_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/ModularIO.ModularIO_C",
		"/FicsItNetworks/Schematics/Research_ModularIO.Research_ModularIO_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Schematics/Storage.Storage_C",
		"/FicsItNetworks/Schematics/Research_DigitalStorage.Research_DigitalStorage_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Computer.Computer_C",
		"/FicsItNetworks/Buildings/Computer/ComputerCase/Build_ComputerCase.Build_ComputerCase_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Drives/Desc_DriveT1.Desc_DriveT1_C",
		"/FicsItNetworks/Items/Drives/Desc_DriveT1.Desc_DriveT1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Floppy/Desc_FloppyDisk.Desc_FloppyDisk_C",
		"/FicsItNetworks/Items/Floppy/Desc_FloppyDisk.Desc_FloppyDisk_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/EEPROM/EEPROM_Lua_Desc.EEPROM_Lua_Desc_C",
		"/FicsItNetworks/Items/EEPROM/Desc_EEPROM_Text.Desc_EEPROM_Text_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Items/EEPROM/Desc_EEPROM_Lua.Desc_EEPROM_Lua_C",
		"/FicsItNetworks/Items/EEPROM/Desc_EEPROM_Text.Desc_EEPROM_Text_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/RAM/RAM_T1.RAM_T1_C",
		"/FicsItNetworks/Buildings/Computer/Modules/RAM/Build_RAM_T1.Build_RAM_T1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/NetworkCard/NetworkCard.NetworkCard_C",
		"/FicsItNetworks/Buildings/Computer/Modules/NetworkCard/Build_NetworkCard.Build_NetworkCard_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/InternetCard/InternetCard.InternetCard_C",
		"/FicsItNetworks/Buildings/Computer/Modules/InternetCard/Build_InternetCard.Build_InternetCard_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/DriveHolder/DriveHolder.DriveHolder_C",
		"/FicsItNetworks/Buildings/Computer/Modules/DriveHolder/Build_DriveHolder.Build_DriveHolder_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/Screen/ScreenDriver.ScreenDriver_C",
		"/FicsItNetworks/Buildings/Computer/Modules/Screen/Build_ScreenDriver.Build_ScreenDriver_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/GPU/GPU_T1.GPU_T1_C",
		"/FicsItNetworks/Buildings/Computer/Modules/GPU_T1/Build_GPU_T1.Build_GPU_T1_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/GPU/GPU_T2.GPU_T2_C",
		"/FicsItNetworks/Buildings/Computer/Modules/GPU_T2/Build_GPU_T2.Build_GPU_T2_C");
	FIN_CoreRedirect(Type_Class,
		"/FicsItNetworks/Computer/Modules/CPU/CPU_Lua.CPU_Lua_C",
		"/FicsItNetworks/Buildings/Computer/Modules/CPU_Lua/Build_CPU_Lua.Build_CPU_Lua_C");
	// End v0.3.21

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

	FCoreRedirects::AddRedirectList(redirects, "FIN-Code");
	
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR && UE_GAME
		// Copy FS UUID Item Context Menu Entry //
		UClass* Slot = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Interface/UI/InGame/InventorySlots/Widget_InventorySlot.Widget_InventorySlot_C"));
		check(Slot);
		UFunction* Function = Slot->FindFunctionByName(TEXT("CreateSplitSlider"));
		UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
		HookManager->HookBlueprintFunction(Function, InventorSlot_CreateWidgetSlider_Hook, EPredefinedHookOffset::Return);
		UClass* NodeInfo = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Interface/UI/InGame/MAMTree/Widget_MAMTree_NodeInfo.Widget_MAMTree_NodeInfo_C"));
		Function = NodeInfo->FindFunctionByName(TEXT("Can Research"));
		HookManager->HookBlueprintFunction(Function, &ResearchNodeInfoWidget_CanResearch_Hook, EPredefinedHookOffset::Return);
		Function = NodeInfo->FindFunctionByName(TEXT("UpdateState"));
		HookManager->HookBlueprintFunction(Function, &ResearchNodeInfoWidget_UpdateState_Hook, EPredefinedHookOffset::Return);

		SUBSCRIBE_UOBJECT_METHOD(AFGBuildGun, BeginPlay, [](auto& scope, AFGBuildGun* BuildGun) {
			if (!BuildGun) return;
			AFINBuildgunHooks* hooks = BuildGun->GetWorld()->GetSubsystem<USubsystemActorManager>()->GetSubsystemActor<AFINBuildgunHooks>();
			if (hooks) BuildGun->mOnRecipeSampled.AddUniqueDynamic(hooks, &AFINBuildgunHooks::OnRecipeSampled);
		});
#endif
	});
}
UE_ENABLE_OPTIMIZATION_SHIP

void FFicsItNetworksModule::ShutdownModule() {
	FFINStyle::Shutdown();
}

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
