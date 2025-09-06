#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "FGBuildableDoor.h"
#include "FGCentralStorageContainer.h"
#include "FGCentralStorageSubsystem.h"
#include "FGGameState.h"
#include "FGIconLibrary.h"
#include "FGResourceSinkSubsystem.h"
#include "FIRSourceStaticHooks.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableResourceSink.h"

class FFIRDoorHelper {
public:
	static void AFGBuildableDoor_Update(AFGBuildableDoor* InDoor, EDoorConfiguration InConfig) {
		InDoor->OnRep_DoorConfiguration();
	}
};

BeginClass(AFGBuildableResourceSink, "ResourceSink", "Resource Sink", "The resource sink, also known a A.W.E.S.O.M.E Sink.\nPoints are saved in multiple different tracks.\n* 0 = Default (Ressources)\n* 1 = Exploration (DNA Points)")
BeginProp(RInt, numCoupons, "Num Coupons", "The number of available coupons to print.") {
	FIRReturn (int64)AFGResourceSinkSubsystem::Get(self)->GetNumCoupons();
} EndProp()
BeginFunc(getNumPoints, "Get Num Points", "Get the number of available points for a given track.") {
	InVal(0, RInt, track, "Track", "The track you want to get the number of points for.")
	OutVal(1, RInt, numPoints, "Num Points", "The number of points for the given track.")
	Body()
	numPoints = (int64)AFGResourceSinkSubsystem::Get(self)->GetNumTotalPoints((EResourceSinkTrack) FMath::Clamp(track, 0, (int64)EResourceSinkTrack::RST_MAX - 1));
} EndFunc()
BeginFunc(getNumPointsToNextCoupon, "Get Num Points To Next Coupon", "Get the number of needed points for the next coupon for a given track.") {
	InVal(0, RInt, track, "Track", "The track you want to get the number of points till the next coupon.")
	OutVal(1, RInt, numPoints, "Num Points", "The number of points needed for the next coupon for the given track.")
	Body()
	numPoints = (int64)AFGResourceSinkSubsystem::Get(self)->GetNumPointsToNextCoupon((EResourceSinkTrack) FMath::Clamp(track, 0, (int64)EResourceSinkTrack::RST_MAX - 1));
} EndFunc()
BeginFunc(getCouponProgress, "Get Coupon Progress", "Get the percentage of the progress for the next coupon for the given track.") {
	InVal(0, RInt, track, "Track", "The track you want to get the number of points till the next coupon.")
	OutVal(1, RFloat, progress, "Progress", "The percentage of the progress for the next coupon for the given track.")
	Body()
	progress = AFGResourceSinkSubsystem::Get(self)->GetProgressionTowardsNextCoupon((EResourceSinkTrack) FMath::Clamp(track, 0, (int64)EResourceSinkTrack::RST_MAX - 1));
} EndFunc()
BeginStaticProp(RInt, trackDefault, "Track Default", "The default track for the resource sink.") {
	FIRReturn (int64)EResourceSinkTrack::RST_Default;
} EndProp()
BeginStaticProp(RInt, trackExploration, "Track Exploration", "The exploration track for the resource sink.") {
	FIRReturn (int64)EResourceSinkTrack::RST_Exploration;
} EndProp()
EndClass()

BeginClass(AFGBuildableDoor, "Door", "Door", "The base class of all doors.")
BeginFunc(getConfiguration, "Get Configuration", "Returns the Door Mode/Configuration.\n0 = Automatic\n1 = Always Closed\n2 = Always Open") {
	OutVal(0, RInt, configuration, "Configuration", "The current door mode/configuration.")
	Body()
	configuration = (FIRInt)self->GetmDoorConfiguration();
} EndFunc()
BeginFunc(setConfiguration, "Set Configuration", "Sets the Door Mode/Configuration, only some modes are allowed, if the mod you try to set is invalid, nothing changes.\n0 = Automatic\n1 = Always Closed\n2 = Always Open", 0) {
	InVal(0, RInt, configuration, "Configuration", "The new configuration for the door.")
	Body()
	EDoorConfiguration Config = (EDoorConfiguration)FMath::Clamp(configuration, 0, 2);
	self->SetmDoorConfiguration(Config);
	FFIRDoorHelper::AFGBuildableDoor_Update(self, Config);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightSource, "LightSource", "Light Source", "The base class for all light you can build.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the light is enabled") {
	FIRReturn self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the light should automatically turn on and off depending on the time of the day.") {
	FIRReturn self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the light.") {
	return self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the light uses.") {
	FIRReturn (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
} EndProp()
BeginFunc(getColorFromSlot, "Get Color from Slot", "Returns the light color that is referenced by the given slot.") {
	InVal(0, RInt, slot, "Slot", "The slot you want to get the referencing color from.")
	OutVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot references.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	color = (FIRStruct) SubSys->GetBuildableLightColorSlot(slot);
} EndFunc()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(0, RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	auto gameState = Cast<AFGGameState>(self->GetWorld()->GetGameState());
	gameState->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightsControlPanel, "LightsControlPanel", "Light Source", "A control panel to configure multiple lights at once.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the lights should be enabled") {
	FIRReturn self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightEnabled(Val);
	}
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the lights should automatically turn on and off depending on the time of the day.") {
	FIRReturn self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the lights.") {
	FIRReturn self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the lights should use.") {
	return (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(0, RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginStruct(FIconData, "IconData", "Icon Data", "A struct containing information about a game icon (used in f.e. signs).")
BeginProp(RBool, isValid, "Is Valid", "True if the icon data refers to an valid icon") {
	FIRReturn FIRBool(self->ID >= 0);
} EndProp()
BeginProp(RInt, id, "ID", "The icon ID.") {
	FIRReturn (FIRInt)self->ID;
} EndProp()
BeginProp(RString, ref, "Ref", "The media reference of this icon.") {
	FIRReturn FString::Printf(TEXT("icon:%i"), self->ID);
} EndProp()
BeginProp(RBool, animated, "Animated", "True if the icon is animated.") {
	FIRReturn self->Animated;
} EndProp()
BeginProp(RString, iconName, "Icon Name", "The name of the icon.") {
	FIRReturn self->IconName.ToString();
} EndProp()
BeginProp(RString, iconType, "Icon Type", "The type of the icon.\n0 = Building\n1 = Part\n2 = Equipment\n3 = Monochrome\n4 = Material\n5 = Custom\n6 = Map Stamp") {
	FIRReturn (FIRInt)self->IconType;
} EndProp()
BeginProp(RBool, hidden, "Hidden", "True if the icon is hidden in the selection.") {
	FIRReturn self->Hidden;
} EndProp()
BeginProp(RBool, searchOnly, "Search Only", "True if the icon will be shown in selection only if searched for directly by name.") {
	FIRReturn self->SearchOnly;
} EndProp()
EndStruct()

BeginClass(AFGCentralStorageContainer, "DimensionalDepotUploader", "Dimensional Depot Uploader", "The container that allows you to upload items to the dimensional depot. The dimensional depot is also known as central storage.")
BeginProp(RBool, isUploadingToCentralStorage, "Is Uploading To Central Storage", "True if the uploader is currently uploading items to the dimensional depot.") {
	FIRReturn self->IsUploadingToCentralStorage();
} EndProp()
BeginProp(RFloat, centralStorageUploadProgress, "Central Storage Upload Progress", "The upload progress of the item that currently gets uploaded.") {
	FIRReturn self->GetCentralStorageUploadProgress();
} EndProp()
BeginProp(RBool, isUploadInventoryEmpty, "Is Upload Inventory Empty", "True if the inventory of items to upload is empty.") {
	FIRReturn self->IsUploadInventoryEmpty();
} EndProp()
BeginProp(RTrace<AFGCentralStorageSubsystem>, centralStorage, "Central Storage", "The central stroage it self.") {
	FIRReturn (FIRAny)(Ctx.GetTrace() / AFGCentralStorageSubsystem::Get(self));
} EndProp()
EndClass()

BeginClass(AFGCentralStorageSubsystem, "DimensionalDepot", "Dimensional Depot", "The dimensional depot, remote storage or also known as central storage.")
Hook(UFIRDimensionalDepotHook)
BeginProp(RInt, centralStorageItemStackLimit, "Central Storage Item Stack Limit", "The stack limit of the central storage.") {
	FIRReturn (FIRInt)self->GetCentralStorageItemStackLimit();
} EndProp()
BeginProp(RFloat, centralStorageTimeToUpload, "Central Storage Time to Upload", "The amount of time it takes to upload an item to the central storage.") {
	FIRReturn (FIRInt)self->GetCentralStorageTimeToUpload();
} EndProp()
BeginFunc(getItemCountFromCentralStorage, "Get Item Count from Central Storage", "Returns the number of items of a given type that is stored within the central storage.") {
	InVal(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item you want to get the number of items in the central storage from.")
	OutVal(1, RInt, number, "Number", "The number of items in the central storage.")
	Body()
	number = (FIRInt) self->GetNumItemsFromCentralStorage(itemType);
} EndFunc()
BeginFunc(getAllItemsFromCentralStorage, "Get all Items from Cental Stroage", "Return a list of all items the central storage currently contains.") {
	OutVal(0, RArray<RStruct<FItemAmount>>, items, "Items", "The list of items that the central storage currently contains.")
	Body()
	TArray<FItemAmount> itemAmounts;
	self->GetAllItemsFromCentralStorage(itemAmounts);
	FIRArray itemsAny;
	for (const FItemAmount& item : itemAmounts) {
		itemsAny.Add((FIRStruct)item);
	}
	items = itemsAny;
} EndFunc()
BeginFunc(canUploadItemsToCentralStorage, "Can upload Items to Central Storage", "Returns true if any items of the given type can be uploaded to the central storage.") {
	InVal(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item you want to check if it can be uploaded.")
	OutVal(1, RBool, canUpload, "Can Upload", "True if the given item type can be uploaded to the central storage.")
	Body()
	canUpload = self->CanUploadItemsToCentralStorage(itemType);
} EndFunc()
BeginFunc(getCentralStorageItemLimit, "Get Central Storage Item Limit", "Returns the maxiumum number of items of a given type you can upload to the central storage.") {
	InVal(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item you want to check if it can be uploaded.")
	OutVal(1, RInt, number, "Number", "The maximum number of items you can upload.")
	Body()
	number = (FIRInt) self->GetCentralStorageItemLimit(itemType);
} EndFunc()
BeginSignal(NewItemAdded, "New Item Added", "Gets triggered when a new item gets uploaded to the central storage.") {
	SignalParam(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item that got uploaded.")
} EndSignal()
BeginSignal(ItemAmountUpdated, "Item Amount Updated", "Gets triggered when the amount of item in the central storage changes.") {
	SignalParam(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item that got uploaded.")
	SignalParam(1, RInt, itemAmount, "Item Amount", "The new amount of items of the given type.")
} EndSignal()
BeginSignal(ItemLimitReachedUpdated, "Item Limit Reached Updated", "Gets triggered when an item type reached maximum capacity, or when it now has again available space for new items.") {
	SignalParam(0, RClass<UFGItemDescriptor>, itemType, "Item Type", "The type of the item which changed if it has reached the limit or not.")
	SignalParam(1, RBool, reached, "Reached", "True if the given item type has reached the limit or not.")
} EndSignal()
EndClass()
