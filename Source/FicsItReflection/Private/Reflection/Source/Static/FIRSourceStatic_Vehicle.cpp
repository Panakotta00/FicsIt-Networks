#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/ReflectionHelper.h"

#include "FGHealthComponent.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Reflection/Source/Static/FIRTargetPoint.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "WheeledVehicles/FGWheeledVehicleIdentifier.h"

class FFIRVehicleHelper {
public:
	static bool IsAutopilotEnabled(AFGWheeledVehicle* Vehicle) {
		AFGWheeledVehicleIdentifier* Identifier = Vehicle ? Vehicle->GetVehicleIdentifier() : nullptr;
		return Identifier && Identifier->IsAutopilotEnabled();
	}

	static void SetAutopilotEnabled(AFGWheeledVehicle* Vehicle, bool bEnabled) {
		AFGWheeledVehicleIdentifier* Identifier = Vehicle ? Vehicle->GetVehicleIdentifier() : nullptr;
		if (Identifier) Identifier->SetAutopilotEnabled(bEnabled);
	}

	static int32 GetCurrentTargetIndex(AFGWheeledVehicle* Vehicle) {
		AFGWheeledVehicleIdentifier* Identifier = Vehicle ? Vehicle->GetVehicleIdentifier() : nullptr;
		return Identifier ? Identifier->GetCurrentTargetWaypointIndex() : INDEX_NONE;
	}

	static void SetCurrentTargetIndex(AFGWheeledVehicle* Vehicle, int32 Index) {
		AFGWheeledVehicleIdentifier* Identifier = Vehicle ? Vehicle->GetVehicleIdentifier() : nullptr;
		if (Identifier) Identifier->SetCurrentTargetWaypoint(Index);
	}

	static AFGTargetPoint* FindTargetByIndex(AFGDrivingTargetList* List, int64 Index) {
		if (!List || Index < 0) return nullptr;
		int64 CurrentIndex = 0;
		for (AFGTargetPoint* CurrentTarget = List->mFirst; CurrentTarget; CurrentTarget = CurrentTarget->mNext) {
			if (CurrentIndex == Index) return CurrentTarget;
			if (CurrentTarget == List->mLast) break;
			++CurrentIndex;
		}
		return nullptr;
	}

	static int64 FindTargetIndex(AFGDrivingTargetList* List, const AFGTargetPoint* Target) {
		if (!List || !Target) return INDEX_NONE;
		int64 CurrentIndex = 0;
		for (AFGTargetPoint* CurrentTarget = List->mFirst; CurrentTarget; CurrentTarget = CurrentTarget->mNext) {
			if (CurrentTarget == Target) return CurrentIndex;
			if (CurrentTarget == List->mLast) break;
			++CurrentIndex;
		}
		return INDEX_NONE;
	}

	static int32 GetTargetCount(AFGDrivingTargetList* List) {
		int32 Count = 0;
		if (!List) return Count;
		for (AFGTargetPoint* CurrentTarget = List->mFirst; CurrentTarget; CurrentTarget = CurrentTarget->mNext) {
			++Count;
			if (CurrentTarget == List->mLast) break;
		}
		return Count;
	}

	static void InsertItem(AFGDrivingTargetList* List, AFGTargetPoint* Target, AFGTargetPoint* After) {
		if (!List || !Target) return;
		Target->mNext = nullptr;
		if (!List->mFirst) {
			List->mFirst = Target;
			List->mLast = Target;
			return;
		}
		if (!After) After = List->mLast;
		Target->mNext = After->mNext;
		After->mNext = Target;
		if (List->mLast == After || !Target->mNext) List->mLast = Target;
	}

	static void RemoveItem(AFGDrivingTargetList* List, AFGTargetPoint* Target) {
		if (!List || !Target) return;
		AFGTargetPoint* Previous = nullptr;
		for (AFGTargetPoint* CurrentTarget = List->mFirst; CurrentTarget; CurrentTarget = CurrentTarget->mNext) {
			if (CurrentTarget == Target) {
				if (Previous) Previous->mNext = CurrentTarget->mNext;
				else List->mFirst = CurrentTarget->mNext;
				if (List->mLast == CurrentTarget) List->mLast = Previous;
				CurrentTarget->mNext = nullptr;
				return;
			}
			if (CurrentTarget == List->mLast) break;
			Previous = CurrentTarget;
		}
	}
};

BeginClass(AFGVehicle, "Vehicle", "Vehicle", "A base class for all vehicles.")
	BeginProp(RFloat, health, "Health", "The health of the vehicle.") {
	FIRReturn 0.0f;
} EndProp()
BeginProp(RFloat, maxHealth, "Max Health", "The maximum amount of health this vehicle can have.") {
	FIRReturn 0.0f;
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the vehicle is currently self driving.") {
	FIRReturn false;
} EndProp()
EndClass()

BeginClass(AFGWheeledVehicle, "WheeledVehicle", "Wheeled Vehicle", "The base class for all vehicles that used wheels for movement.")
BeginProp(RBool, isAutopilotEnabled, "Is Autopilot Enabled", "True if the vehicle is currently auto piloting.", 0) {
	FIRReturn FFIRVehicleHelper::IsAutopilotEnabled(self);
} PropSet() {
	FFIRVehicleHelper::SetAutopilotEnabled(self, Val);
} EndProp()
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
	isValid = true;
} EndFunc()
BeginFunc(getCurrentTarget, "Get Current Target", "Returns the index of the target that the vehicle tries to move to right now.") {
	OutVal(0, RInt, index, "Index", "The index of the current target.")
	Body()
	index = (int64)FFIRVehicleHelper::GetCurrentTargetIndex(self);
} EndFunc()
BeginFunc(setCurrentTarget, "Set Current Target", "Sets the target with the given index as the target this vehicle tries to move to right now.") {
	InVal(0, RInt, index, "Index", "The index of the target this vehicle should move to now.")
	Body()
	AFGDrivingTargetList* List = self->mTargetList;
	AFGTargetPoint* Target = FFIRVehicleHelper::FindTargetByIndex(List, index);
	if (!Target) throw FFIRException("index out of range");
	FFIRVehicleHelper::SetCurrentTargetIndex(self, index);
} EndFunc()
BeginFunc(getTargetList, "Get Target List", "Returns the list of targets/path waypoints.") {
	OutVal(0, RTrace<AFGDrivingTargetList>, targetList, "Target List", "The list of targets/path-waypoints.")
	Body()
	targetList = Ctx.GetTrace() / self->mTargetList;
} EndFunc()
BeginProp(RFloat, speed, "Speed", "The current forward speed of this vehicle.") {
	FIRReturn self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, burnRatio, "Burn Ratio", "The amount of fuel this vehicle burns.") {
	FIRReturn self->GetManualFuelConsumption();
} EndProp()
BeginProp(RBool, hasFuel, "Has Fuel", "True if the vehicle has currently fuel to drive.") {
	FIRReturn self->HasFuel();
} EndProp()
EndClass()

BeginClass(AFGDrivingTargetList, "TargetList", "Target List", "The list of targets/path-waypoints a autonomous vehicle can drive")
BeginFunc(getTarget, "Get Target", "Returns the target struct at with the given index in the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target you want to get the struct from.")
	OutVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The TargetPoint-Struct with the given index in the target list.")
	Body()
	AFGTargetPoint* Target = FFIRVehicleHelper::FindTargetByIndex(self, index);
	if (!Target) throw FFIRException("index out of range");
	target = (FIRAny)FFIRTargetPoint(Target);
} EndFunc()
BeginFunc(removeTarget, "Remove Target", "Removes the target with the given index from the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to remove from the target list.")
	Body()
	AFGTargetPoint* Target = FFIRVehicleHelper::FindTargetByIndex(self, index);
	if (!Target) throw FFIRException( "index out of range");
	FFIRVehicleHelper::RemoveItem(self, Target);
	Target->Destroy();
} EndFunc()
BeginFunc(addTarget, "Add Target", "Adds the given target point struct at the end of the target list.") {
	InVal(0, RStruct<FFIRTargetPoint>, target, "Target", "The target point you want to add.")
	Body()
	AFGTargetPoint* Target = target.ToWheeledTargetPoint(self);
	if (!Target) throw FFIRException("failed to create target");
	FFIRVehicleHelper::InsertItem(self, Target, self->mLast);
} EndFunc()
BeginFunc(setTarget, "Set Target", "Allows to set the target at the given index to the given target point struct.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to update with the given target point struct.")
	InVal(1, RStruct<FFIRTargetPoint>, target, "Target", "The new target point struct for the given index.")
	Body()
	AFGTargetPoint* Target = FFIRVehicleHelper::FindTargetByIndex(self, index);
	if (!Target) throw FFIRException("index out of range");
	Target->SetActorLocation(target.Pos);
	Target->SetActorRotation(target.Rot);
	Target->mTargetSpeed = FMath::RoundToInt(target.Speed);
	Target->mWaitTime = target.Wait;
} EndFunc()
BeginFunc(getTargets, "Get Targets", "Returns a list of target point structs of all the targets in the target point list.") {
	OutVal(0, RArray<RStruct<FFIRTargetPoint>>, targets, "Targets", "A list of target point structs containing all the targets of the target point list.")
	Body()
	TArray<FIRAny> Targets;
	AFGTargetPoint* CurrentTarget = self->mFirst;
	while (CurrentTarget) {
		Targets.Add((FIRAny)FFIRTargetPoint(CurrentTarget));
		if (CurrentTarget == self->mLast) break;
		CurrentTarget = CurrentTarget->mNext;
	}
	targets = Targets;
} EndFunc()
BeginFunc(setTargets, "Set Targets", "Removes all targets from the target point list and adds the given array of target point structs to the empty target point list.", 0) {
	InVal(0, RArray<RStruct<FFIRTargetPoint>>, targets, "Targets", "A list of target point structs you want to place into the empty target point list.")
	Body()
	int Count = FFIRVehicleHelper::GetTargetCount(self);
	for (const FIRAny& Target : targets) {
		FFIRVehicleHelper::InsertItem(self, Target.GetStruct().Get<FFIRTargetPoint>().ToWheeledTargetPoint(self), self->mLast);
	}
	for (int i = 0; i < Count; ++i) {
		AFGTargetPoint* Target = self->mFirst;
		FFIRVehicleHelper::RemoveItem(self, Target);
		if (Target) Target->Destroy();
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
