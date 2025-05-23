﻿#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/Source/Static/FIRSourceStaticHooks.h"

#include "FGPowerCircuit.h"
#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildableCircuitBridge.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildablePowerStorage.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"

BeginClass(UFGPowerConnectionComponent, "PowerConnection", "Power Connection", "A actor component that allows for a connection point to the power network. Basically a point were a power cable can get attached to.")
	BeginProp(RInt, connections, "Connections", "The amount of connections this power connection has.") {
	FIRReturn (int64)self->GetNumConnections();
} EndProp()
BeginProp(RInt, maxConnections, "Max Connections", "The maximum amount of connections this power connection can handle.") {
	FIRReturn (int64)self->GetMaxNumConnections();
} EndProp()
BeginFunc(getPower, "Get Power", "Returns the power info component of this power connection.") {
	OutVal(0, RTrace<UFGPowerInfoComponent>, power, "Power", "The power info compoent this power connection uses.")
	Body()
	power = Ctx.GetTrace() / self->GetPowerInfo();
} EndFunc();
BeginFunc(getCircuit, "Get Circuit", "Returns the power circuit to which this connection component is attached to.") {
	OutVal(0, RTrace<UFGPowerCircuit>, circuit, "Circuit", "The Power Circuit this connection component is attached to.")
	Body()
	circuit = Ctx.GetTrace() / self->GetPowerCircuit();
} EndFunc()
EndClass()

BeginClass(UFGPowerInfoComponent, "PowerInfo", "Power Info", "A actor component that provides information and mainly statistics about the power connection it is attached to.")
BeginProp(RFloat, dynProduction, "Dynamic Production", "The production cpacity this connection provided last tick.") {
	FIRReturn self->GetRegulatedDynamicProduction();
} EndProp()
BeginProp(RFloat, baseProduction, "Base Production", "The base production capactiy this connection always provides.") {
	FIRReturn self->GetBaseProduction();
} EndProp()
BeginProp(RFloat, maxDynProduction,	"Max Dynamic Production", "The maximum production capactiy this connection could have provided to the circuit in the last tick.") {
	FIRReturn self->GetDynamicProductionCapacity();
} EndProp()
BeginProp(RFloat, targetConsumption, "Target Consumption", "The amount of energy the connection wanted to consume from the circuit in the last tick.") {
	FIRReturn  self->GetTargetConsumption();
} EndProp()
BeginProp(RFloat, consumption, "Consumption", "The amount of energy the connection actually consumed in the last tick.") {
	FIRReturn  self->GetBaseProduction();
} EndProp();
BeginProp(RBool, hasPower, "Has Power", "True if the connection has satisfied power values and counts as beeing powered. (True if it has power)") {
	FIRReturn  self->HasPower();
} EndProp();
BeginFunc(getCircuit, "Get Circuit", "Returns the power circuit this info component is part of.") {
	OutVal(0, RTrace<UFGPowerCircuit>, circuit, "Circuit", "The Power Circuit this info component is attached to.")
	Body()
	circuit = Ctx.GetTrace() / self->GetPowerCircuit();
}
EndFunc()
EndClass()

BeginClass(UFGPowerCircuit, "PowerCircuit", "Power Circuit", "A Object that represents a whole power circuit.")
Hook(UFIRPowerCircuitHook)
BeginSignal(PowerFuseChanged, "Power Fuse Changed", "Get Triggered when the fuse state of the power circuit changes.")
EndSignal()
BeginFunc(resetFuse, "Reset Fuse", "Resets the fuse of this circuit", 0) {
	Body()
	self->ResetFuse();
}EndFunc()
BeginProp(RFloat, production, "Production", "The amount of power produced by the whole circuit in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	FIRReturn  stats.PowerProduced;
} EndProp()
BeginProp(RFloat, consumption, "Consumption", "The power consumption of the whole circuit in thge last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	FIRReturn  stats.PowerConsumed;
} EndProp()
BeginProp(RFloat, capacity, "Capacity", "The power capacity of the whole network in the last tick. (The max amount of power available in the last tick)") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	FIRReturn  stats.PowerProductionCapacity;
} EndProp()
BeginProp(RFloat, batteryInput, "Battery Input", "The power that gone into batteries in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	FIRReturn  stats.BatteryPowerInput;
} EndProp()
BeginProp(RFloat, maxPowerConsumption, "Max Power Consumption", "The maximum consumption of power in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	FIRReturn  stats.MaximumPowerConsumption;
} EndProp()
BeginProp(RBool, isFuesed, "Is Fuesed", "True if the fuse in the network triggered.") {
	FIRReturn  self->IsFuseTriggered();
} EndProp()
BeginProp(RBool, hasBatteries, "Has Batteries", "True if the power circuit has batteries connected to it.") {
	FIRReturn  self->HasBatteries();
} EndProp()
BeginProp(RFloat, batteryCapacity, "Battery Capacity", "The energy capacity all batteries of the network combined provide.") {
	FIRReturn  self->GetBatterySumPowerStoreCapacity();
} EndProp()
BeginProp(RFloat, batteryStore, "Battery Store", "The amount of energy currently stored in all battereies of the network combined.") {
	FIRReturn  self->GetBatterySumPowerStore();
} EndProp()
BeginProp(RFloat, batteryStorePercent, "Battery Store Percentage", "The fill status in percent of all battereies of the network combined.") {
	FIRReturn  self->GetBatterySumPowerStorePercent();
} EndProp()
BeginProp(RFloat, batteryTimeUntilFull, "Battery Time until Full", "The time in seconds until every battery in the network is filled.") {
	FIRReturn  self->GetTimeToBatteriesFull();
} EndProp()
BeginProp(RFloat, batteryTimeUntilEmpty, "Battery Time until Empty", "The time in seconds until every battery in the network is empty.") {
	FIRReturn  self->GetTimeToBatteriesEmpty();
} EndProp()
BeginProp(RFloat, batteryIn, "Battery Input", "The amount of energy that currently gets stored in every battery of the whole network.") {
	FIRReturn  self->GetBatterySumPowerInput();
} EndProp()
BeginProp(RFloat, batteryOut, "Battery Output", "The amount of energy that currently discharges from every battery in the whole network.") {
	FIRReturn  self->GetBatterySumPowerOutput();
} EndProp()
EndClass()

BeginClass(AFGBuildablePowerStorage, "PowerStorage", "Power Storage", "A building that can store power for later usage.")
BeginProp(RFloat, powerStore, "Power Store", "The current amount of energy stored in the storage.") {
	FIRReturn  self->GetPowerStore();
} EndProp()
BeginProp(RFloat, powerCapacity, "Power Capacity", "The amount of energy the storage can hold max.") {
	FIRReturn  self->GetPowerStoreCapacity();
} EndProp()
BeginProp(RFloat, powerStorePercent, "Power Store Percent", "The current power store in percent.") {
	FIRReturn  self->GetPowerStorePercent();
} EndProp()
BeginProp(RFloat, powerIn, "Power Input", "The amount of power coming into the storage.") {
	FIRReturn  self->GetPowerInput();
} EndProp()
BeginProp(RFloat, powerOut, "Power Output", "The amount of power going out from the storage.") {
	FIRReturn  self->GetPowerOutput();
} EndProp()
BeginProp(RFloat, timeUntilFull, "Time until Full", "The time in seconds until the storage is filled.") {
	FIRReturn  self->GetTimeUntilFull();
} EndProp()
BeginProp(RFloat, timeUntilEmpty, "Time until Empty", "The time in seconds until the storage is empty.") {
	FIRReturn  self->GetTimeUntilEmpty();
} EndProp()
BeginProp(RInt, batteryStatus, "Battery Status", "The current status of the battery.\n0 = Idle, 1 = Idle Empty, 2 = Idle Full, 3 = Power In, 4 = Power Out") {
	FIRReturn  (int64) self->GetBatteryStatus();
} EndProp()
BeginProp(RInt, batteryMaxIndicatorLevel, "Max Indicator Level", "The maximum count of Level lights that are shown.") {
	FIRReturn  (int64) self->GetIndicatorLevelMax();
} EndProp()
EndClass()

BeginClass(AFGBuildableCircuitBridge, "CircuitBridge", "Circuite Bridget", "A building that can connect two circuit networks together.")
BeginProp(RBool, isBridgeConnected, "Is Bridge Connected", "True if the bridge is connected to two circuits.") {
	FIRReturn  self->IsBridgeConnected();
} EndProp()
BeginProp(RBool, isBridgeActive, "Is Bridge Active", "True if the two circuits are connected to each other and act as one entity.") {
	FIRReturn  self->IsBridgeActive();
} EndProp()
EndClass()

BeginClass(AFGBuildableCircuitSwitch, "CircuitSwitch", "Circuit Switch", "A circuit bridge that can be activated and deactivate by the player.")
BeginProp(RBool, hasBuildingTag, "Has Building Tag", "Returns true if this building is taggable") {
	FIRReturn self->Execute_HasBuildingTag(self);
} EndProp()
BeginProp(RString, buildingTag, "Building Tag", "Returns the building tag if it has any") {
	FIRReturn self->Execute_GetBuildingTag(self);
} EndProp()
BeginProp(RBool, isSwitchOn, "Is Switch On", "True if the two circuits are connected to each other and act as one entity.", 0) {
	FIRReturn  self->IsSwitchOn();
} EndProp()
BeginFunc(setIsSwitchOn, "Set Is Switch On", "Changes the circuit switch state.", 0) {
	InVal(0, RBool, state, "State", "The new switch state.")
	Body()
	self->SetSwitchOn(state);
} EndFunc()
EndClass()

BeginClass(AFGBuildablePriorityPowerSwitch, "CircuitSwitchPriority", "Circuit Priority Switch", "A circuit power switch that can be activated and deactivated based on a priority to prevent a full factory power shutdown.")
BeginProp(RInt, priority, "Priority", "The priority group of which this switch is part of.") {
	FIRReturn  (FIRInt)self->GetPriority();
} PropSet() {
	self->SetPriority(Val);
} EndProp()
EndClass()
