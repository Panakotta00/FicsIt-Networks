#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "FGBuildableDoor.h"
#include "FGGameState.h"
#include "FGResourceSinkSubsystem.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableResourceSink.h"

static void AFGBuildableDoor_Update(AFGBuildableDoor* InDoor, EDoorConfiguration InConfig) {
	InDoor->OnRep_DoorConfiguration();
}

BeginClass(AFGBuildableResourceSink, "ResourceSink", "Resource Sink", "The resource sink, also known a A.W.E.S.O.M.E Sink")
BeginProp(RInt, numPoints, "Num Points", "The number of available points.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumTotalPoints(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RInt, numCoupons, "Num Coupons", "The number of available coupons to print.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumCoupons();
} EndProp()
BeginProp(RInt, numPointsToNextCoupon, "Num Points To Next Coupon", "The number of needed points for the next coupon.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumPointsToNextCoupon(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RFloat, couponProgress, "Coupon Progress", "The percentage of the progress for the next coupon.") {
	Return AFGResourceSinkSubsystem::Get(self)->GetProgressionTowardsNextCoupon(EResourceSinkTrack::RST_Default);
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
	AFGBuildableDoor_Update(self, Config);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightSource, "LightSource", "Light Source", "The base class for all light you can build.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the light is enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the light should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
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
	return (int64) self->GetLightControlData().ColorSlotIndex;
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
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightsControlPanel, "LightsControlPanel", "Light Source", "A control panel to configure multiple lights at once.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the lights should be enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightEnabled(Val);
	}
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the lights should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the lights.") {
	return self->GetLightControlData().Intensity;
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
	Return FIRBool(self->ID >= 0);
} EndProp()
BeginProp(RInt, id, "ID", "The icon ID.") {
	Return (FIRInt)self->ID;
} EndProp()
BeginProp(RString, ref, "Ref", "The media reference of this icon.") {
	Return FString::Printf(TEXT("icon:%i"), self->ID);
} EndProp()
BeginProp(RBool, animated, "Animated", "True if the icon is animated.") {
	Return self->Animated;
} EndProp()
BeginProp(RString, iconName, "Icon Name", "The name of the icon.") {
	Return self->IconName.ToString();
} EndProp()
BeginProp(RString, iconType, "Icon Type", "The type of the icon.\n0 = Building\n1 = Part\n2 = Equipment\n3 = Monochrome\n4 = Material\n5 = Custom\n6 = Map Stamp") {
	Return (FIRInt)self->IconType;
} EndProp()
BeginProp(RBool, hidden, "Hidden", "True if the icon is hidden in the selection.") {
	Return self->Hidden;
} EndProp()
BeginProp(RBool, searchOnly, "Search Only", "True if the icon will be shown in selection only if searched for directly by name.") {
	Return self->SearchOnly;
} EndProp()
EndStruct()
