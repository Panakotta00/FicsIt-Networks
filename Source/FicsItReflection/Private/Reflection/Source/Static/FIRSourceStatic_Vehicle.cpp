#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/ReflectionHelper.h"

#include "FGHealthComponent.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

BeginClass(AFGVehicle, "Vehicle", "Vehicle", "A base class for all vehicles.")
	BeginProp(RFloat, health, "Health", "The health of the vehicle.") {
	Return self->GetHealthComponent()->GetCurrentHealth();
} EndProp()
BeginProp(RFloat, maxHealth, "Max Health", "The maximum amount of health this vehicle can have.") {
	Return self->GetHealthComponent()->GetMaxHealth();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the vehicle is currently self driving.") {
	Return self->IsSelfDriving();
} PropSet() {
	FReflectionHelper::SetPropertyValue<FBoolProperty>(self, TEXT("mIsSelfDriving"), Val);
} EndProp()
EndClass()

BeginClass(AFGWheeledVehicle, "WheeledVehicle", "Wheeled Vehicle", "The base class for all vehicles that used wheels for movement.")
BeginFunc(getFuelInv, "Get Fuel Inventory", "Returns the inventory that contains the fuel of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the vehicle.")
	Body()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getStorageInv, "Get Storage Inventory", "Returns the inventory that contains the storage of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The storage inventory of the vehicle.")
	Body()
	inventory = Ctx.GetTrace() / self->GetStorageInventory();
} EndFunc()
BeginFunc(isValidFuel, "Is Valid Fuel", "Allows to check if the given item type is a valid fuel for this vehicle.") {
	InVal(0, RClass<UFGItemDescriptor>, item, "Item", "The item type you want to check.")
	OutVal(1, RBool, isValid, "Is Valid", "True if the given item type is a valid fuel for this vehicle.")
	Body()
	isValid = self->IsValidFuel(item);
} EndFunc()

BeginFunc(getCurrentTarget, "Get Current Target", "Returns the index of the target that the vehicle tries to move to right now.") {
	OutVal(0, RInt, index, "Index", "The index of the current target.")
	Body()
	AFGDrivingTargetList* List = self->GetTargetList();
	index = (int64)List->FindTargetIndex(List->mCurrentTarget);
} EndFunc()
BeginFunc(nextTarget, "Next Target", "Sets the current target to the next target in the list.") {
	Body()
	self->PickNextTarget();
} EndFunc()
BeginFunc(setCurrentTarget, "Set Current Target", "Sets the target with the given index as the target this vehicle tries to move to right now.") {
	InVal(0, RInt, index, "Index", "The index of the target this vehicle should move to now.")
	Body()
	AFGDrivingTargetList* List = self->GetTargetList();
	AFGTargetPoint* Target = List->FindTargetByIndex(index);
	if (!Target) throw FFIRReflectionException("index out of range");
	List->mCurrentTarget = Target;
} EndFunc()
BeginFunc(getTargetList, "Get Target List", "Returns the list of targets/path waypoints.") {
	OutVal(0, RTrace<AFGDrivingTargetList>, targetList, "Target List", "The list of targets/path-waypoints.")
	Body()
	targetList = Ctx.GetTrace() / self->GetTargetList();
} EndFunc()
BeginProp(RFloat, speed, "Speed", "The current forward speed of this vehicle.") {
	Return self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, burnRatio, "Burn Ratio", "The amount of fuel this vehicle burns.") {
	Return self->GetFuelBurnRatio();
} EndProp()
BeginProp(RBool, hasFuel, "Has Fuel", "True if the vehicle has currently fuel to drive.") {
	Return self->HasFuel();
} EndProp()
EndClass()

BeginClass(AFGDrivingTargetList, "TargetList", "Target List", "The list of targets/path-waypoints a autonomous vehicle can drive")
BeginFunc(getTarget, "Get Target", "Returns the target struct at with the given index in the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target you want to get the struct from.")
	OutVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The TargetPoint-Struct with the given index in the target list.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRReflectionException("index out of range");
	target = (FIRAny)FFIRTargetPoint(Target);
} EndFunc()
BeginFunc(removeTarget, "Remove Target", "Removes the target with the given index from the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to remove from the target list.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRReflectionException( "index out of range");
	self->RemoveItem(Target);
	Target->Destroy();
} EndFunc()
BeginFunc(addTarget, "Add Target", "Adds the given target point struct at the end of the target list.") {
	InVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The target point you want to add.")
	Body()
	AFGTargetPoint* Target = target.ToWheeledTargetPoint(self);
	if (!Target) throw FFIRReflectionException("failed to create target");
	self->InsertItem(Target, self->mLast);
} EndFunc()
BeginFunc(setTarget, "Set Target", "Allows to set the target at the given index to the given target point struct.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to update with the given target point struct.")
	InVal(1, RStruct<FFIRTargetPoint>, target, "Target", "The new target point struct for the given index.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFIRReflectionException("index out of range");
	Target->SetActorLocation(target.Pos);
	Target->SetActorRotation(target.Rot);
	Target->SetTargetSpeed(target.Speed);
	Target->SetWaitTime(target.Wait);
} EndFunc()
BeginFunc(getTargets, "Get Targets", "Returns a list of target point structs of all the targets in the target point list.") {
	OutVal(0, RArray<RStruct<FFIRTargetPoint>>, targets, "Targets", "A list of target point structs containing all the targets of the target point list.")
	Body()
	TArray<FIRAny> Targets;
	AFGTargetPoint* CurrentTarget = nullptr;
	int i = 0;
	do {
		if (i++) CurrentTarget = CurrentTarget->GetNext();
		else CurrentTarget = self->GetFirstTarget();
		Targets.Add((FIRAny)FFIRTargetPoint(CurrentTarget));
	} while (CurrentTarget && CurrentTarget != self->GetLastTarget());
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
EndClass()

BeginClass(AFGBuildableDockingStation, "DockingStation", "Docking Station", "A docking station for wheeled vehicles to transfer cargo.")
BeginFunc(getFuelInv, "Get Fueld Inventory", "Returns the fuel inventory of the docking station.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the docking station.")
	Body()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getInv, "Get Inventory", "Returns the cargo inventory of the docking staiton.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The cargo inventory of this docking station.")
	Body()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getDocked, "Get Docked", "Returns the currently docked actor.") {
	OutVal(0, RTrace<AActor>, docked, "Docked", "The currently docked actor.")
	Body()
	docked = Ctx.GetTrace() / self->GetDockedActor();
} EndFunc()
BeginFunc(undock, "Undock", "Undocked the currently docked vehicle from this docking station.") {
	Body()
	self->Undock(true);
} EndFunc()
BeginProp(RBool, isLoadMode, "Is Load Mode", "True if the docking station loads docked vehicles, flase if it unloads them.") {
	Return self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isLoadUnloading, "Is Load Unloading", "True if the docking station is currently loading or unloading a docked vehicle.") {
	Return self->IsLoadUnloading();
} EndProp()
EndClass()
