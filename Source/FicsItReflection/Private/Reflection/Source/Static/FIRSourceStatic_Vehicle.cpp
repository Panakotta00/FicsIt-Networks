#include "Reflection/ReflectionHelper.h"

#include "FGHealthComponent.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Reflection/Source/Static/FIRTargetPoint.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

#include "Reflection/Source/FIRSourceStaticMacros.h"
#include "WheeledVehicles/FGVehicleAutopilotComponent.h"
#include "WheeledVehicles/FGVehiclePathNode.h"
#include "WheeledVehicles/FGVehicleSubsystem.h"
#include "WheeledVehicles/FGWheeledVehicleIdentifier.h"

class FFIRVehicleHelper {
public:
	static void SetTarget(AFGWheeledVehicle* Vehicle, AFGTargetPoint* Target) {
		//Vehicle->GetInfo()->GetSimulationMovement()->SetTarget(Target, false);
	}

	static bool isValidFuel(AFGWheeledVehicle* vehicle, TSubclassOf<UFGItemDescriptor> item, int32 index) {
		return vehicle->FilterFuelClasses(item, index);
	}
};

BeginClass(AFGVehicle, "Vehicle", "Vehicle", "A base class for all vehicles.")
EndClass()

BeginClass(AFGWheeledVehicle, "WheeledVehicle", "Wheeled Vehicle", "The base class for all vehicles that used wheels for movement.")
BeginProp(RBool, isAutopilotEnabled, "Is Autopilot Enabled", "True if the vehicle is currently auto piloting.", 0) {
	if (auto identifier = self->GetVehicleIdentifier()) {
		FIRReturn identifier->IsAutopilotEnabled();
	} else {
		FIRReturn false;
	}
} PropSet() {
	if (auto identifier = self->GetVehicleIdentifier()) {
		if (identifier->CanEnableAutopilot()) {
			identifier->SetAutopilotEnabled(Val);
		}
	}
} EndProp()
BeginFunc(getFuelInv, "Get Fuel Inventory", "Returns the inventory that contains the fuel of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the vehicle.")
	FIRBody()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getStorageInv, "Get Storage Inventory", "Returns the inventory that contains the storage of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The storage inventory of the vehicle.")
	FIRBody()
	inventory = Ctx.GetTrace() / self->GetStorageInventory();
} EndFunc()
BeginFunc(isValidFuel, "Is Valid Fuel", "Allows to check if the given item type is a valid fuel for this vehicle.") {
	InVal(0, RClass<UFGItemDescriptor>, item, "Item", "The item type you want to check.")
	OutVal(1, RBool, isValid, "Is Valid", "True if the given item type is a valid fuel for this vehicle.")
	FIRBody()
	isValid = FFIRVehicleHelper::isValidFuel(self, item, 0);
} EndFunc()
	BeginFunc(getCurrentTarget, "Get Current Target", "Returns the index of the target that the vehicle tries to move to right now.") {
	OutVal(0, RString, nodeGuid, "Node Guide", "The GUID of the current target.")
	FIRBody()
	auto pathNode = self->GetVehicleIdentifier()->GetCurrentFromPathNodeGUID();
	nodeGuid = pathNode.ToString();
} EndFunc()
BeginFunc(setCurrentTarget, "Set Current Target", "Sets the target with the given index as the target this vehicle tries to move to right now.") {
	InVal(0, RInt, index, "Index", "The index of the target this vehicle should move to now.")
	FIRBody()
	if (auto identifier = self->GetVehicleIdentifier()) {
		if (!identifier->GetVehicleRoute().IsValidIndex(index)) throw FFIRException("index out of range");
		identifier->SetCurrentTargetWaypoint(index);
	}
} EndFunc()
BeginFunc(getVehicleRoute, "Get Vehicle Route", "Returns the list of waypoints GUIDs.") {
	OutVal(0, RArray<RString>, vehicleRoute, "Vehicle Route", "The list of targets/path-waypoint GUIDs.")
	FIRBody()
	if (auto identifier = self->GetVehicleIdentifier()) {
		TArray<FIRAny> route;
		for (auto guid : identifier->GetVehicleRoute()) {
			route.Add(guid.ToString());
		}
		vehicleRoute = route;
	}
} EndFunc()
BeginFunc(getWaypointPosition, "Get Waypoint Position", "Returns the position of the waypoint with the given GUID.") {
	InVal(0, RString, waypoint, "Waypoint", "The GUID of the waypoint.")
	OutVal(1, RStruct<FVector>, position, "Position", "The position of the waypoint with the given GUID.")
	FIRBody()
	FGuid guid;
	FGuid::Parse(waypoint, guid);
	if (auto node = AFGVehicleSubsystem::Get(self)->FindReplicatedPathNodeByGuid(guid)) {
		position = (FIRAny) node->GetActorLocation();
	}
} EndFunc()
// TODO: Add Vehicle Route Editing
BeginProp(RFloat, speed, "Speed", "The current forward speed of this vehicle.") {
	FIRReturn self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, fuelConsumptionAutopilot, "Fuel Consumption Autopilot", "The amount of fuel this vehicle burns when driven by autopilot.") {
	FIRReturn self->GetAutopilotFuelConsumption();
} EndProp()
BeginProp(RFloat, fuelConsumptionManual, "Fuel Consumption Manual", "The amount of fuel this vehicle burns when driven by manually.") {
	FIRReturn self->GetManualFuelConsumption();
} EndProp()
BeginProp(RBool, hasFuel, "Has Fuel", "True if the vehicle has currently fuel to drive.") {
	FIRReturn self->HasFuel();
} EndProp()
EndClass()

// TODO: Add Vehicle Network exploration
/*BeginClass(AFGDrivingTargetList, "TargetList", "Target List", "The list of targets/path-waypoints a autonomous vehicle can drive")
BeginFunc(getTarget, "Get Target", "Returns the target struct at with the given index in the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target you want to get the struct from.")
	OutVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The TargetPoint-Struct with the given index in the target list.")
	FIRBody()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRException("index out of range");
	target = (FIRAny)FFIRTargetPoint(Target);
} EndFunc()
BeginFunc(removeTarget, "Remove Target", "Removes the target with the given index from the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to remove from the target list.")
	FIRBody()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRException( "index out of range");
	self->RemoveItem(Target);
	Target->Destroy();
} EndFunc()
BeginFunc(addTarget, "Add Target", "Adds the given target point struct at the end of the target list.") {
	InVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The target point you want to add.")
	FIRBody()
	AFGTargetPoint* Target = target.ToWheeledTargetPoint(self);
	if (!Target) throw FFIRException("failed to create target");
	self->InsertItem(Target, self->mLast);
} EndFunc()
BeginFunc(setTarget, "Set Target", "Allows to set the target at the given index to the given target point struct.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to update with the given target point struct.")
	InVal(1, RStruct<FFIRTargetPoint>, target, "Target", "The new target point struct for the given index.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRException("index out of range");
	Target->SetActorLocation(target.Pos);
	Target->SetActorRotation(target.Rot);
	Target->SetTargetSpeed(target.Speed);
	Target->SetWaitTime(target.Wait);
} EndFunc()
BeginFunc(getTargets, "Get Targets", "Returns a list of target point structs of all the targets in the target point list.") {
	OutVal(0, RArray<RStruct<FFIRTargetPoint>>, targets, "Targets", "A list of target point structs containing all the targets of the target point list.")
	Body()
	TArray<FIRAny> Targets;
	AFGTargetPoint* CurrentTarget = self->GetFirstTarget();
	while (CurrentTarget) {
		Targets.Add((FIRAny)FFIRTargetPoint(CurrentTarget));
		if (CurrentTarget == self->GetLastTarget()) break;
		CurrentTarget = CurrentTarget->GetNext();
	}
	targets = Targets;
} EndFunc()
BeginFunc(setTargets, "Set Targets", "Removes all targets from the target point list and adds the given array of target point structs to the empty target point list.", 0) {
	InVal(0, RArray<RStruct<FFIRTargetPoint>>, targets, "Targets", "A list of target point structs you want to place into the empty target point list.")
	Body()
	int Count = self->GetTargetCount();
	for (const FIRAny& Target : targets) {
		self->InsertItem(Target.GetStruct().Get<FFIRTargetPoint>().ToWheeledTargetPoint(self), self->mLast);
	}
	for (int i = 0; i < Count; ++i) {
		self->RemoveItem(self->mFirst);
	}
} EndFunc()
EndClass()*/

BeginClass(AFGBuildableDockingStation, "DockingStation", "Docking Station", "A docking station for wheeled vehicles to transfer cargo.")
BeginFunc(getFuelInv, "Get Fuel Inventory", "Returns the fuel inventory of the docking station.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the docking station.")
	FIRBody()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getInv, "Get Inventory", "Returns the cargo inventory of the docking staiton.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The cargo inventory of this docking station.")
	FIRBody()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getDocked, "Get Docked", "Returns the currently docked actor.") {
	OutVal(0, RTrace<AActor>, docked, "Docked", "The currently docked actor.")
	FIRBody()
	docked = Ctx.GetTrace() / self->GetDockedActor();
} EndFunc()
BeginFunc(undock, "Undock", "Undocked the currently docked vehicle from this docking station.") {
	FIRBody()
	self->ForceUndockActor();
} EndFunc()
BeginProp(RBool, isLoadMode, "Is Load Mode", "True if the docking station loads docked vehicles, flase if it unloads them.") {
	FIRReturn self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isLoadUnloading, "Is Load Unloading", "True if the docking station is currently loading or unloading a docked vehicle.") {
	FIRReturn self->IsLoadUnloading();
} EndProp()
EndClass()
