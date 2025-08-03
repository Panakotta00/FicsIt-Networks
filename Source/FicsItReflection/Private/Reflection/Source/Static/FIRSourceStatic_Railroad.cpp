#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/ReflectionHelper.h"
#include "Reflection/Source/Static/FIRTrackGraph.h"
#include "Reflection/Source/Static/FIRRailroadSignalBlock.h"
#include "Reflection/Source/Static/FIRSourceStaticHooks.h"
#include "Reflection/Source/Static/FIRTargetPoint.h"
#include "Reflection/Source/Static/FIRTimeTableStop.h"

#include "FGRailroadTimeTable.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "FGLocomotive.h"
#include "FGTrain.h"
#include "FGTrainPlatformConnection.h"
#include "FGTrainStationIdentifier.h"
#include "FicsItLogLibrary.h"
#include "FIRSubsystem.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "Buildables/FGBuildableRailroadSwitchControl.h"
#include "Buildables/FGBuildableTrainPlatform.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"

class FIRRailroadHelper {
public:
	static TArray<TWeakObjectPtr<AFGRailroadVehicle>> FFGRailroadSignalBlock_GetOccupiedBy(const FFGRailroadSignalBlock& Block) {
		return Block.mOccupiedBy;
	}

	static TArray<TSharedPtr<FFGRailroadBlockReservation>> FFGRailroadSignalBlock_GetQueuedReservations(const FFGRailroadSignalBlock& Block) {
		return Block.mPendingReservations;
	}

	static TArray<TSharedPtr<FFGRailroadBlockReservation>> FFGRailroadSignalBlock_GetApprovedReservations(const FFGRailroadSignalBlock& Block) {
		return Block.mApprovedReservations;
	}

	static UFGTrainPlatformConnection* AFGBuildableTrainPlatform_mPlatformConnection0(AFGBuildableTrainPlatform* self) {
		return self->mPlatformConnection0;
	}
};

BeginClass(AFGBuildableTrainPlatform, "TrainPlatform", "Train Platform", "The base class for all train station parts.")
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this platform is part of.") {
	OutVal(0, RStruct<FFIRTrackGraph>, graph, "Graph", "The track graph of which this platform is part of.")
	Body()
	graph = (FIRAny)FFIRTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this train platform is placed.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
	OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
	OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
	Body()
	FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFIRException("Railroad track position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getConnectedPlatform, "Get Connected Platform", "Returns the connected platform in the given direction.") {
	InVal(0, RObject<UFGTrainPlatformConnection>, platformConnection, "Platform Connection", "The platform connection of which you want to find the opposite connection of.")
	OutVal(1, RTrace<UFGTrainPlatformConnection>, oppositeConnection, "Opposite Connection", "The platform connection at the opposite side.")
	Body()
	oppositeConnection = Ctx.GetTrace() / self->GetConnectionInOppositeDirection(platformConnection.Get());
} EndFunc()
BeginFunc(getAllConnectedPlatforms, "Get all connected Platforms", "Returns a list of all connected platforms in order.") {
	OutVal(1, RArray<RTrace<UFGTrainPlatformConnection>>, platforms, "Platforms", "The list of connected platforms")
	Body()
	TArray<FIRAny> connectedPlatforms;
	//UFGTrainPlatformConnection* firstConnection = self->GetComponentByClass<UFGTrainPlatformConnection>();
	UFGTrainPlatformConnection* firstConnection = FIRRailroadHelper::AFGBuildableTrainPlatform_mPlatformConnection0(self);
	UFGTrainPlatformConnection* otherConnection = self->GetConnectionInOppositeDirection(firstConnection);
	UFGTrainPlatformConnection* connected = otherConnection->GetConnectedTo();
	connectedPlatforms.Add(Ctx.GetTrace());
	while (IsValid(connected)) {
		AFGBuildableTrainPlatform* platform = connected->GetPlatformOwner();
		connectedPlatforms.Insert(Ctx.GetTrace() / platform, 0);
		connected = platform->GetConnectionInOppositeDirection(connected);
		if (!IsValid(connected)) break;
		connected = connected->GetConnectedTo();
	}
	connected = firstConnection->GetConnectedTo();
	while (IsValid(connected)) {
		AFGBuildableTrainPlatform* platform = connected->GetPlatformOwner();
		connectedPlatforms.Add(Ctx.GetTrace() / platform);
		connected = platform->GetConnectionInOppositeDirection(connected);
		if (!IsValid(connected)) break;
		connected = connected->GetConnectedTo();
	}
	platforms = connectedPlatforms;
} EndFunc()
BeginFunc(getDockedVehicle, "Get Docked Vehicle", "Returns the currently docked vehicle.") {
	OutVal(0, RTrace<AFGVehicle>, vehicle, "Vehicle", "The currently docked vehicle")
	Body()
	vehicle = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockedRailroadVehicle"));
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master platform of this train station.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, master, "Master", "The master platform of this train station.")
	Body()
	master = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mStationDockingMaster"));
} EndFunc()
BeginFunc(getDockedLocomotive, "Get Docked Locomotive", "Returns the currently docked locomotive at the train station.") {
	OutVal(0, RTrace<AFGLocomotive>, locomotive, "Locomotive", "The currently docked locomotive at the train station.")
	Body()
	locomotive = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockingLocomotive"));
} EndFunc()
BeginProp(RInt, status, "Status", "The current docking status of the platform.") {
	FIRReturn (int64)self->GetDockingStatus();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the orientation of the platform is reversed relative to the track/station.") {
	FIRReturn self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(UFGTrainPlatformConnection, "TrainPlatformConnection", "Train Platform Connection", "A component that is used to connect two Train Platforms together.")
BeginProp(RTrace<UFGTrainPlatformConnection>, connected, "Connected", "The connected train platform connection.") {
	FIRReturn (Ctx.GetTrace() / self->GetConnectedTo());
} EndProp()
BeginProp(RTrace<UFGRailroadTrackConnectionComponent>, trackConnection, "Track Connected", "The associated railroad track connection.") {
	FIRReturn (Ctx.GetTrace() / self->GetRailroadConnectionReference());
} EndProp()
BeginProp(RTrace<AFGBuildableTrainPlatform>, platformOwner, "Platform Owner", "The train platform that owns this platform connection component.") {
	FIRReturn (Ctx.GetTrace() / self->GetPlatformOwner());
} EndProp()
BeginProp(RInt, connectionType, "Connection Type", "The type of this train platform connection.\n0 = Out\n1 = In\n2 = Neutral") {
	FIRReturn (Ctx.GetTrace() / self->GetPlatformOwner());
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadStation, "RailroadStation", "Railroad Station", "The train station master platform. This platform holds the name and manages docking of trains.")
Hook(UFIRRailroadStationHook)
BeginSignal(StartDocking, "Start Docking", "Triggers when a train tries to dock onto the station.")
	SignalParam(0, RBool, successful, "Successful", "True if the train successfully docked.")
	SignalParam(1, RTrace<AFGLocomotive>, locomotive, "Locomotive", "The locomotive that tries to dock onto the station.")
	SignalParam(2, RFloat, offset, "Offset", "The offset at witch the train tried to dock.")
EndSignal()
BeginSignal(FinishDocking, "Finish Docking", "Triggers when a train finished the docking procedure and is ready to depart the station.")
EndSignal()
BeginSignal(CancelDocking, "Cancel Docking", "Triggers when a train cancels the docking procedure.")
EndSignal()
BeginProp(RString, name, "Name", "The name of the railroad station.") {
	FIRReturn self->GetStationIdentifier()->GetStationName().ToString();
} PropSet() {
	self->GetStationIdentifier()->SetStationName(FText::FromString(Val));
} EndProp()
BeginProp(RInt, dockedOffset, "Docked Offset", "The Offset to the beginning of the station at which trains dock.") {
	FIRReturn self->GetDockedVehicleOffset();
} EndProp()
EndClass()

BeginClass(AFGBuildableTrainPlatformCargo, "TrainPlatformCargo", "Train Platform Cargo", "A train platform that allows for loading and unloading cargo cars.")
BeginProp(RBool, isLoading, "Is Loading", "True if the cargo platform is currently loading the docked cargo vehicle.") {
	UFILogLibrary::Log(FIL_Verbosity_Warning, TEXT("TrainPlatformCargo's `isLoading` property is Deprecated! Please use `isInLoadMode` instead."));
	FIRReturn self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isInLoadMode, "Is in Load Mode", "True if the cargo platform is set to load cargo, false if it should unload the cargo.") {
	FIRReturn self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isUnloading, "Is Unloading", "True if the cargo platform is currently loading or unloading the docked cargo vehicle.") {
	FIRReturn self->IsLoadUnloading();
} EndProp()
BeginProp(RFloat, dockedOffset, "Docked Offset", "The offset to the track start of the platform at were the vehicle docked.") {
	FIRReturn self->GetDockedVehicleOffset();
} EndProp()
BeginProp(RFloat, outputFlow, "Output Flow", "The current output flow rate.") {
	FIRReturn self->GetOutflowRate();
} EndProp()
BeginProp(RFloat, inputFlow, "Input Flow", "The current input flow rate.") {
	FIRReturn self->GetInflowRate();
} EndProp()
BeginProp(RBool, fullLoad, "Full Load", "True if the docked cargo vehicle is fully loaded.") {
	FIRReturn (bool)self->IsFullLoad();
} EndProp()
BeginProp(RBool, fullUnload, "Full Unload", "Ture if the docked cargo vehicle is fully unloaded.") {
	FIRReturn (bool)self->IsFullUnload();
} EndProp()
EndClass()

BeginClass(AFGRailroadVehicle, "RailroadVehicle", "Railroad Vehicle", "The base class for any vehicle that drives on train tracks.")
BeginFunc(getTrain, "Get Train", "Returns the train of which this vehicle is part of.") {
	OutVal(0, RTrace<AFGTrain>, train, "Train", "The train of which this vehicle is part of")
	Body()
	train = Ctx.GetTrace() / Cast<UObject>(self->GetTrain());
} EndFunc()
BeginFunc(isCoupled, "Is Coupled", "Allows to check if the given coupler is coupled to another car.") {
	InVal(0, RInt, coupler, "Coupler", "The Coupler you want to check. 0 = Front, 1 = Back")
	OutVal(1, RBool, coupled, "Coupled", "True of the give coupler is coupled to another car.")
	Body()
	coupled = self->IsCoupledAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getCoupled, "Get Coupled", "Allows to get the coupled vehicle at the given coupler.") {
	InVal(0, RInt, coupler, "Coupler", "The Coupler you want to get the car from. 0 = Front, 1 = Back")
	OutVal(1, RTrace<AFGRailroadVehicle>, coupled, "Coupled", "The coupled car of the given coupler is coupled to another car.")
	Body()
	coupled = Ctx.GetTrace() / self->GetCoupledVehicleAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this vehicle is part of.") {
	OutVal(0, RStruct<FFIRTrackGraph>, track, "Track", "The track graph of which this vehicle is part of.")
	Body()
	track = (FIRAny)FFIRTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this vehicle is.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFIRException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getMovement, "Get Movement", "Returns the vehicle movement of this vehicle.") {
	OutVal(0, RTrace<UFGRailroadVehicleMovementComponent>, movement, "Movement", "The movement of this vehicle.")
	Body()
	movement = Ctx.GetTrace() / self->GetRailroadVehicleMovementComponent();
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of this vehicle on the track.") {
	FIRReturn self->GetLength();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if this vehicle is currently docked to a platform.") {
	FIRReturn self->IsDocked();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the vheicle is placed reversed on the track.") {
	FIRReturn self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(UFGRailroadVehicleMovementComponent, "RailroadVehicleMovement", "Railroad Vehicle Movement", "This actor component contains all the infomation about the movement of a railroad vehicle.")
BeginFunc(getVehicle, "Get Vehicle", "Returns the vehicle this movement component holds the movement information of.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, vehicle, "Vehicle", "The vehicle this movement component holds the movement information of.")
	Body()
	vehicle = Ctx.GetTrace() / self->GetOwningRailroadVehicle();
} EndFunc()
BeginFunc(getWheelsetRotation, "Get Wheelset Rotation", "Returns the current rotation of the given wheelset.") {
	InVal(0, RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the rotation of.")
	OutVal(1, RFloat, x, "X", "The wheelset's rotation X component.")
	OutVal(2, RFloat, y, "Y", "The wheelset's rotation Y component.")
	OutVal(3, RFloat, z, "Z", "The wheelset's rotation Z component.")
	Body()
	FVector rot = self->GetWheelsetRotation(wheelset);
	x = rot.X;
	y = rot.Y;
	z = rot.Z;
} EndFunc()
BeginFunc(getWheelsetOffset, "Get Wheelset Offset", "Returns the offset of the wheelset with the given index from the start of the vehicle.") {
	InVal(0, RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the offset of.")
	OutVal(1, RFloat, offset, "Offset", "The offset of the wheelset.")
	Body()
	offset = self->GetWheelsetOffset(wheelset);
} EndFunc()
BeginFunc(getCouplerRotationAndExtention, "Get Coupler Rotation And Extention", "Returns the normal vector and the extention of the coupler with the given index.") {
	InVal(0, RInt, coupler, "Coupler", "The index of which you want to get the normal and extention of.")
	OutVal(1, RFloat, x, "X", "The X component of the coupler normal.")
	OutVal(2, RFloat, y, "Y", "The Y component of the coupler normal.")
	OutVal(3, RFloat, z, "Z", "The Z component of the coupler normal.")
	OutVal(4, RFloat, extention, "Extention", "The extention of the coupler.")
	Body()
	float extension;
	FVector rotation = self->GetCouplerRotationAndExtention(coupler, extension);
	x =rotation.X;
	y = rotation.Y;
	z = rotation.Z;
	extention = extension;
} EndFunc()

BeginProp(RFloat, orientation, "Orientation", "The orientation of the vehicle") {
	FIRReturn self->GetOrientation();
} EndProp()
BeginProp(RFloat, mass, "Mass", "The current mass of the vehicle.") {
	FIRReturn self->GetMass();
} EndProp()
BeginProp(RFloat, tareMass, "Tare Mass", "The tare mass of the vehicle.") {
	FIRReturn self->GetTareMass();
} EndProp()
BeginProp(RFloat, payloadMass, "Payload Mass", "The mass of the payload of the vehicle.") {
	FIRReturn self->GetPayloadMass();
} EndProp()
BeginProp(RFloat, speed, "Speed", "The current forward speed of the vehicle.") {
	FIRReturn self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, relativeSpeed, "Relative Speed", "The current relative forward speed to the ground.") {
	FIRReturn self->GetRelativeForwardSpeed();
} EndProp()
BeginProp(RFloat, maxSpeed, "Max Speed", "The maximum forward speed the vehicle can reach.") {
	FIRReturn self->GetMaxForwardSpeed();
} EndProp()
BeginProp(RFloat, gravitationalForce, "Gravitationl Force", "The current gravitational force acting on the vehicle.") {
	FIRReturn self->GetGravitationalForce();
} EndProp()
BeginProp(RFloat, tractiveForce, "Tractive Force", "The current tractive force acting on the vehicle.") {
	FIRReturn self->GetTractiveForce();
} EndProp()
BeginProp(RFloat, resistiveForce, "Resistive Froce", "The resistive force currently acting on the vehicle.") {
	FIRReturn self->GetResistiveForce();
} EndProp()
BeginProp(RFloat, gradientForce, "Gradient Force", "The gradient force currently acting on the vehicle.") {
	FIRReturn self->GetGradientForce();
} EndProp()
BeginProp(RFloat, brakingForce, "Braking Force", "The braking force currently acting on the vehicle.") {
	FIRReturn self->GetBrakingForce();
} EndProp()
BeginProp(RFloat, airBrakingForce, "Air Braking Force", "The air braking force currently acting on the vehicle.") {
	FIRReturn self->GetAirBrakingForce();
} EndProp()
BeginProp(RFloat, dynamicBrakingForce, "Dynamic Braking Force", "The dynamic braking force currently acting on the vehicle.") {
	FIRReturn self->GetDynamicBrakingForce();
} EndProp()
BeginProp(RFloat, maxTractiveEffort, "Max Tractive Effort", "The maximum tractive effort of this vehicle.") {
	FIRReturn self->GetMaxTractiveEffort();
} EndProp()
BeginProp(RFloat, maxDynamicBrakingEffort, "Max Dynamic Braking Effort", "The maximum dynamic braking effort of this vehicle.") {
	FIRReturn self->GetMaxDynamicBrakingEffort();
} EndProp()
BeginProp(RFloat, maxAirBrakingEffort, "Max Air Braking Effort", "The maximum air braking effort of this vehicle.") {
	FIRReturn self->GetMaxAirBrakingEffort();
} EndProp()
BeginProp(RFloat, trackGrade, "Track Grade", "The current track grade of this vehicle.") {
	FIRReturn self->GetTrackGrade();
} EndProp()
BeginProp(RFloat, trackCurvature, "Track Curvature", "The current track curvature of this vehicle.") {
	FIRReturn self->GetTrackCurvature();
} EndProp()
BeginProp(RFloat, wheelsetAngle, "Wheelset Angle", "The wheelset angle of this vehicle.") {
	FIRReturn self->GetWheelsetAngle();
} EndProp()
BeginProp(RFloat, rollingResistance, "Rolling Resistance", "The current rolling resistance of this vehicle.") {
	FIRReturn self->GetRollingResistance();
} EndProp()
BeginProp(RFloat, curvatureResistance, "Curvature Resistance", "The current curvature resistance of this vehicle.") {
	FIRReturn self->GetCurvatureResistance();
} EndProp()
BeginProp(RFloat, airResistance, "Air Resistance", "The current air resistance of this vehicle.") {
	FIRReturn self->GetAirResistance();
} EndProp()
BeginProp(RFloat, gradientResistance, "Gradient Resistance", "The current gardient resistance of this vehicle.") {
	FIRReturn self->GetGradientResistance();
} EndProp()
BeginProp(RFloat, wheelRotation, "Wheel Rotation", "The current wheel rotation of this vehicle.") {
	FIRReturn self->GetWheelRotation();
} EndProp()
BeginProp(RInt, numWheelsets, "Num Wheelsets", "The number of wheelsets this vehicle has.") {
	FIRReturn (int64)self->GetNumWheelsets();
} EndProp()
BeginProp(RBool, isMoving, "Is Moving", "True if this vehicle is currently moving.") {
	FIRReturn self->IsMoving();
} EndProp()
EndClass()

BeginClass(AFGTrain, "Train", "Train", "This class holds information and references about a trains (a collection of multiple railroad vehicles) and its timetable f.e.")
Hook(UFIRTrainHook)
BeginSignal(SelfDrvingUpdate, "Self Drving Update", "Triggers when the self driving mode of the train changes")
	SignalParam(0, RBool, enabled, "Enabled", "True if the train is now self driving.")
EndSignal()
BeginFunc(getName, "Get Name", "Returns the name of this train.") {
	OutVal(0, RString, name, "Name", "The name of this train.")
	Body()
	name = self->GetTrainName().ToString();
} EndFunc()
BeginFunc(setName, "Set Name", "Allows to set the name of this train.") {
	InVal(0, RString, name, "Name", "The new name of this trian.")
	Body()
	self->SetTrainName(FText::FromString(name));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this train is part of.") {
	OutVal(0, RStruct<FFIRTrackGraph>, track, "Track", "The track graph of which this train is part of.")
	Body()
	track = (FIRAny) FFIRTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(setSelfDriving, "Set Self Driving", "Allows to set if the train should be self driving or not.", 0) {
	InVal(0, RBool, selfDriving, "Self Driving", "True if the train should be self driving.")
	Body()
	self->SetSelfDrivingEnabled(selfDriving);
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master locomotive that is part of this train.") {
	OutVal(0, RTrace<AFGLocomotive>, master, "Master", "The master locomotive of this train.")
	Body()
	master = Ctx.GetTrace() / self->GetMultipleUnitMaster();
} EndFunc()
BeginFunc(getTimeTable, "Get Time Table", "Returns the timetable of this train.") {
	OutVal(0, RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The timetable of this train.")
	Body()
	timeTable = Ctx.GetTrace() / self->GetTimeTable();
} EndFunc()
BeginFunc(newTimeTable, "New Time Table", "Creates and returns a new timetable for this train.", 0) {
	OutVal(0, RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The new timetable for this train.")
	Body()
	timeTable = Ctx.GetTrace() / self->NewTimeTable();
} EndFunc()
BeginFunc(getFirst, "Get First", "Returns the first railroad vehicle that is part of this train.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, first, "First", "The first railroad vehicle that is part of this train.")
	Body()
	first = Ctx.GetTrace() / self->GetFirstVehicle();
} EndFunc()
BeginFunc(getLast, "Get Last", "Returns the last railroad vehicle that is part of this train.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, last, "Last", "The last railroad vehicle that is part of this train.")
	Body()
	last = Ctx.GetTrace() / self->GetLastVehicle();
} EndFunc()
BeginFunc(dock, "Dock", "Trys to dock the train to the station it is currently at.") {
	Body()
	self->Dock();
} EndFunc()
BeginFunc(getVehicles, "Get Vehicles", "Returns a list of all the vehicles this train has.") {
	OutVal(0, RArray<RTrace<AFGRailroadVehicle>>, vehicles, "Vehicles", "A list of all the vehicles this train has.")
	Body()
	TArray<FIRAny> Vehicles;
	for (AFGRailroadVehicle* vehicle : self->mSimulationData.SimulatedVehicles) {
		Vehicles.Add(Ctx.GetTrace() / vehicle);
	}
	vehicles = Vehicles;
} EndFunc()
BeginProp(RBool, isPlayerDriven, "Is Player Driven", "True if the train is currently player driven.") {
	FIRReturn self->IsPlayerDriven();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the train is currently self driving.") {
	FIRReturn self->IsSelfDrivingEnabled();
} EndProp()
BeginProp(RInt, selfDrivingError, "Self Driving Error", "The last self driving error.\n0 = No Error\n1 = No Power\n2 = No Time Table\n3 = Invalid Next Stop\n4 = Invalid Locomotive Placement\n5 = No Path") {
	FIRReturn (int64)self->GetSelfDrivingError();
} EndProp()
BeginProp(RBool, hasTimeTable, "Has Time Table", "True if the train has currently a time table.") {
	FIRReturn self->HasTimeTable();
} EndProp()
BeginProp(RInt, dockState, "Dock State", "The current docking state of the train.") {
	FIRReturn (int64)self->GetDockingState();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if the train is currently docked.") {
	FIRReturn self->IsDocked();
} EndProp()
EndClass()

BeginClass(AFGRailroadTimeTable, "TimeTable", "Time Table", "Contains the time table information of train.")
BeginFunc(addStop, "Add Stop", "Adds a stop to the time table.") {
	InVal(0, RInt, index, "Index", "The zero-based index at which the stop should get added.")
	InVal(1, RTrace<AFGBuildableRailroadStation>, station, "Station", "The railroad station at which the stop should happen.")
	InVal(2, RStruct<FTrainDockingRuleSet>, ruleSet, "Rule Set", "The docking rule set that descibes when the train will depart from the station.")
	OutVal(3, RBool, added, "Added", "True if the stop got sucessfully added to the time table.")
	Body()
	FTimeTableStop stop;
	auto railroadStation = Cast<AFGBuildableRailroadStation>(station.Get());
	if (!IsValid(railroadStation)) {
		throw FFIRException(TEXT("Invalid railroad station"));
	}
	if (index > self->GetNumStops() || index < 0) {
		index = self->GetNumStops();
	}
	stop.Station = railroadStation->GetStationIdentifier();
	stop.DockingRuleSet = ruleSet;

	added = self->AddStop(index, stop);
} EndFunc()
BeginFunc(removeStop, "Remove Stop", "Removes the stop with the given index from the time table.") {
	InVal(0, RInt, index, "Index", "The zero-based index at which the stop should get added.")
	Body()
	self->RemoveStop(index);
} EndFunc()
BeginFunc(getStops, "Get Stops", "Returns a list of all the stops this time table has") {
	OutVal(0, RArray<RStruct<FFIRTimeTableStop>>, stops, "Stops", "A list of time table stops this time table has.")
	Body()
	TArray<FIRAny> Output;
	TArray<FTimeTableStop> Stops;
	self->GetStops(Stops);
	for (const FTimeTableStop& Stop : Stops) {
		Output.Add((FIRAny)FFIRTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet});
	}
	stops = Output;
} EndFunc()
BeginFunc(setStops, "Set Stops", "Allows to empty and fill the stops of this time table with the given list of new stops.") {
	InVal(0, RArray<RStruct<FFIRTimeTableStop>>, stops, "Stops", "The new time table stops.")
	OutVal(1, RBool, gotSet, "Got Set", "True if the stops got sucessfully set.")
	Body()
	TArray<FTimeTableStop> Stops;
	for (const FIRAny& Any : stops) {
		Stops.Add(Any.GetStruct().Get<FFIRTimeTableStop>());
	}
	gotSet = self->SetStops(Stops);
} EndFunc()
BeginFunc(isValidStop, "Is Valid Stop", "Allows to check if the given stop index is valid.") {
	InVal(0, RInt, index, "Index", "The zero-based stop index you want to check its validity.")
	OutVal(1, RBool, valid, "Valid", "True if the stop index is valid.")
	Body()
	valid = self->IsValidStop(index);
} EndFunc()
BeginFunc(getStop, "Get Stop", "Returns the stop at the given index.") {
	InVal(0, RInt, index, "Index", "The zero-based index of the stop you want to get.")
	OutVal(1, RStruct<FFIRTimeTableStop>, stop, "Stop", "The time table stop at the given index.")
	Body()
	FTimeTableStop Stop = self->GetStop(index);
	if (IsValid(Stop.Station)) {
		stop = (FIRAny)FFIRTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet};
	} else {
		stop = FIRAny();
	}
} EndFunc()
BeginFunc(setStop, "Set Stop", "Allows to override a stop already in the time table.") {
	InVal(0, RInt, index, "Index", "The zero-based index of the stop you want to override.")
	InVal(1, RStruct<FFIRTimeTableStop>, stop, "Stop", "The time table stop you want to override with.")
	OutVal(2, RBool, success, "Success", "True if setting was successful, false if not, f.e. invalid index.")
	Body()
	TArray<FTimeTableStop> Stops;
	self->GetStops(Stops);
	if (index < Stops.Num()) {
		Stops[index] = stop;
		self->SetStops(Stops);
		success = true;
	} else {
		success = false;
	}
} EndFunc()
BeginFunc(setCurrentStop, "Set Current Stop", "Sets the stop, to which the train trys to drive to right now.") {
	InVal(0, RInt, index, "Index", "The zero-based index of the stop the train should drive to right now.")
	Body()
	self->SetCurrentStop(index);
} EndFunc()
BeginFunc(incrementCurrentStop, "Increment Current Stop", "Sets the current stop to the next stop in the time table.") {
	Body()
	self->IncrementCurrentStop();
} EndFunc()
BeginFunc(getCurrentStop, "Get Current Stop", "Returns the index of the stop the train drives to right now.") {
	OutVal(0, RInt, index, "Index", "The zero-based index of the stop the train tries to drive to right now.")
    Body()
    index = (int64) self->GetCurrentStop();
} EndFunc()
BeginProp(RInt, numStops, "Num Stops", "The current number of stops in the time table.") {
	FIRReturn (int64)self->GetNumStops();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadTrack, "RailroadTrack", "Railroad Track", "A peice of railroad track over which trains can drive.")
Hook(UFIRRailroadTrackHook)
BeginSignal(VehicleEnter, "VehicleEnter", "Triggered when a vehicle enters the track.")
	SignalParam(0, RTrace<AFGRailroadVehicle>, Vehicle, "Vehicle", "The vehicle that entered the track.")
EndSignal()
BeginSignal(VehicleExit, "VehicleExit", "Triggered when a vehicle exists the track.")
	SignalParam(0, RTrace<AFGRailroadVehicle>, Vehicle, "Vehicle", "The vehicle that exited the track.")
EndSignal()
BeginFunc(getClosestTrackPosition, "Get Closeset Track Position", "Returns the closes track position from the given world position") {
	InVal(0, RStruct<FVector>, worldPos, "World Pos", "The world position form which you want to get the closest track position.")
	OutVal(1, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(2, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(3, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
	FRailroadTrackPosition pos = self->FindTrackPositionClosestToWorldLocation(worldPos);
	if (!pos.IsValid()) throw FFIRException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getWorldLocAndRotAtPos, "Get World Location And Rotation At Position", "Returns the world location and world rotation of the track position from the given track position.") {
	InVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    InVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    InVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    OutVal(3, RStruct<FVector>, location, "Location", "The location at the given track position")
	OutVal(4, RStruct<FVector>, rotation, "Rotation", "The rotation at the given track position (forward vector)")
	Body()
	FRailroadTrackPosition pos(Cast<AFGBuildableRailroadTrack>(track.Get()), offset, forward);
	FVector loc;
	FVector rot;
	self->GetWorldLocationAndDirectionAtPosition(pos, loc, rot);
	location = (FIRAny)loc;
	rotation = (FIRAny)rot;
} EndFunc()
BeginFunc(getConnection, "Get Connection", "Returns the railroad track connection at the given direction.") {
	InVal(0, RInt, direction, "Direction", "The direction of which you want to get the connector from. 0 = front, 1 = back")
	OutVal(1, RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connection component in the given direction.")
	Body()
	connection = Ctx.GetTrace() / self->GetConnection(FMath::Clamp<int>(direction, 0, 1));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this track is part of.") {
	OutVal(0, RStruct<FFIRTrackGraph>, track, "Track", "The track graph of which this track is part of.")
    Body()
    track = (FIRAny)FFIRTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getVehicles, "Get Vehicles", "Returns a list of Railroad Vehicles on the Track") {
	OutVal(0, RArray<RTrace<AFGRailroadVehicle>>, vehicles, "Vehicles", "THe list of vehicles on the track.")
	Body()
	TArray<FIRAny> Vehicles;
	for (AFGRailroadVehicle* vehicle : self->GetVehicles()) {
		Vehicles.Add(Ctx.GetTrace() / vehicle);
	}
	vehicles = Vehicles;
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of the track.") {
	FIRReturn self->GetLength();
} EndProp()
BeginProp(RBool, isOwnedByPlatform, "Is Owned By Platform", "True if the track is part of/owned by a railroad platform.") {
	FIRReturn self->GetIsOwnedByPlatform();
} EndProp()
EndClass()

BeginClass(UFGRailroadTrackConnectionComponent, "RailroadTrackConnection", "Railroad Track Connection", "This is a actor component for railroad tracks that allows to connecto to other track connections and so to connection multiple tracks with each eather so you can build a train network.")
BeginProp(RStruct<FVector>, connectorLocation, "Connector Location", "The world location of the the connection.") {
	FIRReturn self->GetConnectorLocation();
} EndProp()
BeginProp(RStruct<FVector>, connectorNormal, "Connector Normal", "The normal vecotr of the connector.") {
	FIRReturn self->GetConnectorNormal();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected connection with the given index.") {
	InVal(1, RInt, index, "Index", "The index of the connected connection you want to get.")
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connected connection at the given index.")
	Body()
	connection = Ctx.GetTrace() / self->GetConnection(index);
} EndFunc()
BeginFunc(getConnections, "Get Connections", "Returns a list of all connected connections.") {
	OutVal(0, RArray<RTrace<UFGRailroadTrackConnectionComponent>>, connections, "Connections", "A list of all connected connections.")
	Body()
	TArray<FIRAny> Connections;
	for (UFGRailroadTrackConnectionComponent* conn : self->GetConnections()) {
		Connections.Add(Ctx.GetTrace() / conn);
	}
	connections = Connections;
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this connection is.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFIRException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getTrack, "Get Track", "Returns the track of which this connection is part of.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track of which this connection is part of.")
	Body()
	track = Ctx.GetTrace() / self->GetTrack();
} EndFunc()
BeginFunc(getSwitchControl, "Get Switch Control", "Returns the switch control of this connection.") {
	OutVal(0, RTrace<AFGBuildableRailroadSwitchControl>, switchControl, "Switch", "The switch control of this connection.")
	Body()
	switchControl = Ctx.GetTrace() / self->GetSwitchControl();
} EndFunc()
BeginFunc(getStation, "Get Station", "Returns the station of which this connection is part of.") {
	OutVal(0, RTrace<AFGBuildableRailroadStation>, station, "Station", "The station of which this connection is part of.")
	Body()
	station = Ctx.GetTrace() / self->GetStation();
} EndFunc()
BeginFunc(getFacingSignal, "Get Facing Signal", "Returns the signal this connection is facing to.") {
	OutVal(0, RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is facing.")
	Body()
	signal = Ctx.GetTrace() / self->GetFacingSignal();
} EndFunc()
BeginFunc(getTrailingSignal, "Get Trailing Signal", "Returns the signal this connection is trailing from.") {
	OutVal(0, RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is trailing.")
	Body()
	signal = Ctx.GetTrace() / self->GetTrailingSignal();
} EndFunc()
BeginFunc(getOpposite, "Get Opposite", "Returns the opposite connection of the track this connection is part of.") {
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, opposite, "Opposite", "The opposite connection of the track this connection is part of.")
	Body()
	opposite = Ctx.GetTrace() / self->GetOpposite();
} EndFunc()
BeginFunc(getNext, "Get Next", "Returns the next connection in the direction of the track. (used the correct path switched point to)") {
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, next, "Next", "The next connection in the direction of the track.")
	Body()
	next = Ctx.GetTrace() / self->GetNext();
} EndFunc()
BeginFunc(setSwitchPosition, "Set Switch Position", "Sets the position (connection index) to which the track switch points to.") {
	InVal(0, RInt, index, "Index", "The connection index to which the switch should point to.")
	Body()
	self->SetSwitchPosition(index);
} EndFunc()
BeginFunc(getSwitchPosition, "Get Switch Position", "Returns the current switch position.") {
	OutVal(0, RInt, index, "Index", "The index of the connection connection the switch currently points to.")
    Body()
    index = (int64)self->GetSwitchPosition();
} EndFunc()
BeginFunc(forceSwitchPosition, "Force Switch Position", "Forces the switch position to a given location. Even autopilot will be forced to use this track. A negative number can be used to remove the forced track.", 0) {
	InVal(0, RInt, index, "Index", "The connection index to whcih the switch should be force to point to. Negative number to remove the lock.")
	Body()
	self->SetSwitchPosition(index);
	AFIRSubsystem::GetReflectionSubsystem(self)->ForceRailroadSwitch(self, index);
} EndFunc()
BeginProp(RBool, isConnected, "Is Connected", "True if the connection has any connection to other connections.") {
	FIRReturn self->IsConnected();
} EndProp()
BeginProp(RBool, isFacingSwitch, "Is Facing Switch", "True if this connection is pointing to the merge/spread point of the switch.") {
	FIRReturn self->IsFacingSwitch();
} EndProp()
BeginProp(RBool, isTrailingSwitch, "Is Trailing Switch", "True if this connection is pointing away from the merge/spread point of a switch.") {
	FIRReturn self->IsTrailingSwitch();
} EndProp()
BeginProp(RInt, numSwitchPositions, "Num Switch Positions", "Returns the number of different switch poisitions this switch can have.") {
	FIRReturn (int64)self->GetNumSwitchPositions();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadSwitchControl, "RailroadSwitchControl", "Railroad Switch Control", "The controler object for a railroad switch.")
BeginFunc(toggleSwitch, "Toggle Switch", "Toggles the railroad switch like if you interact with it.") {
	Body()
	self->ToggleSwitchPosition();
} EndFunc()
BeginFunc(switchPosition, "Switch Position", "Returns the current switch position of this switch.") {
	OutVal(0, RInt, position, "Position", "The current switch position of this switch.")
    Body()
    position = (int64)self->GetSwitchPosition();
} EndFunc()
BeginFunc(getControlledConnections, "Get Controlled Connections", "Returns the Railroad Connections this switch is controlling.") {
	OutVal(0, RArray<RTrace<UFGRailroadTrackConnectionComponent>>, connections, "Connections", "The controlled connections.")
	Body()
	TArray<FIRAny> Connections;
	for (auto connection : self->GetControlledConnections()) {
		Connections.Add(Ctx.GetTrace() / connection);
	}
	connections = Connections;
} EndFunc()
EndClass()

BeginClass(AFGBuildableRailroadSignal, "RailroadSignal", "Railroad Signal", "A train signal to control trains on a track.")
Hook(UFIRRailroadSignalHook)
BeginSignal(AspectChanged, "Aspect Changed", "Triggers when the aspect of this signal changes.")
	SignalParam(0, RInt, aspect, "Aspect", "The new aspect of the signal (see 'Get Aspect' for more information)")
EndSignal()
BeginSignal(ValidationChanged, "Validation Changed", "Triggers when the validation of this signal changes.")
	SignalParam(0, RInt, validation, "Validation", "The new validation of the signal (see 'Block Validation' for more information)")
EndSignal()
BeginProp(RBool, isPathSignal, "Is Path Signal", "True if this signal is a path-signal.") {
	FIRReturn self->IsPathSignal();
} EndProp()
BeginProp(RBool, isBiDirectional, "Is Bi-Directional", "True if this signal is bi-directional. (trains can pass into both directions)") {
	FIRReturn self->IsBiDirectional();
} EndProp()
BeginProp(RBool, hasObservedBlock, "Has Observed Block", "True if this signal is currently observing at least one block.") {
	FIRReturn self->HasObservedBlock();
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Any error states of the block.\n0 = Unknown\n1 = No Error\n2 = No Exit Signal\n3 = Contains Loop\n4 = Contains Mixed Entry Signals") {
	FIRReturn (int64)self->GetBlockValidation();
} EndProp()
BeginProp(RInt, aspect, "Aspect", "The aspect of the signal. The aspect shows if a train is allowed to pass (clear) or not and if it should dock.\n0 = Unknown\n1 = The track is clear and the train is allowed to pass.\n2 = The next track is Occupied and the train should stop\n3 = The train should dock.") {
	FIRReturn (int64)self->GetAspect();
} EndProp()
BeginFunc(getObservedBlock, "Get Observed Block", "Returns the track block this signals observes.") {
	OutVal(0, RStruct<FFIRRailroadSignalBlock>, block, "Block", "The railroad signal block this signal is observing.")
	Body()
	block = FIRStruct(FFIRRailroadSignalBlock(self->GetObservedBlock()));
} EndFunc()
BeginFunc(getGuardedConnnections, "Get Guarded Connections", "Returns a list of the guarded connections. (incoming connections)") {
	OutVal(0, RArray<RTrace<UFGRailroadTrackConnectionComponent>>, guardedConnections, "GuardedConnections", "The guarded connections.")
	Body()
	TArray<FIRAny> GuardedConnections;
	for (UFGRailroadTrackConnectionComponent* Connection : self->GetGuardedConnections()) {
		GuardedConnections.Add(Ctx.GetTrace() / Connection);
	}
	guardedConnections = GuardedConnections;
} EndFunc()
BeginFunc(getObservedConnections, "Get Observed Connections", "Returns a list of the observed connections. (outgoing connections)") {
	OutVal(0, RArray<RTrace<UFGRailroadTrackConnectionComponent>>, observedConnections, "ObservedConnections", "The observed connections.")
	Body()
	TArray<FIRAny> ObservedConnections;
	for (UFGRailroadTrackConnectionComponent* Connection : self->GetObservedConnections()) {
		ObservedConnections.Add(Ctx.GetTrace() / Connection);
	}
	observedConnections = ObservedConnections;
} EndFunc()
EndClass()

BeginStructConstructable(FTrainDockingRuleSet, "TrainDockingRuleSet", "Train Docking Rule Set", "Contains infromation about the rules that descibe when a trian should depart from a station")
BeginProp(RInt, definition, "Defintion", "0 = Load/Unload Once, 1 = Fully Load/Unload") {
	FIRReturn (FIRInt)self->DockingDefinition;
} PropSet() {
	self->DockingDefinition = (ETrainDockingDefinition)Val;
} EndProp()
BeginProp(RFloat, duration, "Duration", "The amount of time the train will dock at least.") {
	FIRReturn self->DockForDuration;
} PropSet() {
	self->DockForDuration = Val;
} EndProp()
BeginProp(RBool, isDurationAndRule, "Is Duration and Rule", "True if the duration of the train stop and the other rules have to be applied.") {
	FIRReturn self->IsDurationAndRule;
} PropSet() {
	self->IsDurationAndRule = Val;
} EndProp()
BeginFunc(getLoadFilters, "Get Load Filters", "Returns the types of items that will be loaded.") {
	OutVal(0, RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	Body()
	TArray<FIRAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->LoadFilterDescriptors) {
		Filters.Add((FIRClass)Filter);
	}
	filters = Filters;
} EndFunc()
BeginFunc(setLoadFilters, "Set Load Filters", "Sets the types of items that will be loaded.") {
	InVal(0, RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	Body()
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FIRAny& Filter : filters) {
		Filters.Add(Filter.GetClass());
	}
	self->LoadFilterDescriptors = Filters;
} EndFunc()
BeginFunc(getUnloadFilters, "Get Unload Filters", "Returns the types of items that will be unloaded.") {
	OutVal(0, RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	Body()
	TArray<FIRAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->UnloadFilterDescriptors) {
		Filters.Add((FIRClass)Filter);
	}
	filters = Filters;
} EndFunc()
BeginFunc(setUnloadFilters, "Set Unload Filters", "Sets the types of items that will be loaded.") {
	InVal(0, RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	Body()
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FIRAny& Filter : filters) {
		Filters.Add(Filter.GetClass());
	}
	self->UnloadFilterDescriptors = Filters;
} EndFunc()
EndStruct()

BeginStructConstructable(FFIRTimeTableStop, "TimeTableStop", "Time Table Stop", "Information about a train stop in a time table.")
BeginProp(RTrace<AFGBuildableRailroadStation>, station, "Station", "The station at which the train should stop") {
	FIRReturn self->Station;
} PropSet() {
	self->Station = Val;
} EndProp()
BeginFunc(getRuleSet, "Get Rule Set", "Returns The rule set wich describe when the train will depart from the train station.") {
	OutVal(0, RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set of this time table stop.")
	Body()
	ruleset = FIRStruct(self->RuleSet);
} EndFunc()
BeginFunc(setRuleSet, "Set Rule Set", "Allows you to change the Rule Set of this time table stop.") {
	InVal(0, RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set you want to use instead.")
	Body()
	self->RuleSet = ruleset;
} EndFunc()
EndStruct()

BeginStruct(FFIRTrackGraph, "TrackGraph", "Track Graph", "Struct that holds a cache of a whole train/rail network.")
BeginFunc(getTrains, "Get Trains", "Returns a list of all trains in the network.") {
	OutVal(0, RArray<RTrace<AFGTrain>>, trains, "Trains", "The list of trains in the network.")
	Body()
	TArray<FIRAny> Trains;
	TArray<AFGTrain*> TrainList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrains(self->TrackID, TrainList);
	for (AFGTrain* Train : TrainList) {
		Trains.Add(self->Trace / Train);
	}
	trains = Trains;
} EndFunc()
BeginFunc(getStations, "Get Stations", "Returns a list of all trainstations in the network.") {
	OutVal(0, RArray<RTrace<AFGBuildableRailroadStation>>, stations, "Stations", "The list of trainstations in the network.")
    Body()
    TArray<FIRAny> Stations;
	TArray<AFGTrainStationIdentifier*> StationList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrainStations(self->TrackID, StationList);
	for (const auto& Station : StationList) {
		Stations.Add(self->Trace / Station->mStation);
	}
	stations = Stations;
} EndFunc()
EndStruct()

// TODO: 1.0: Redo Railroad Signal Blocks
BeginStruct(FFIRRailroadSignalBlock, "RailroadSignalBlock", "Railroad Signal Block", "A track section that combines the area between multiple signals.")
BeginProp(RBool, isValid, "Is Valid", "Is true if this signal block reference is valid.") {
	FIRReturn self->Block.IsValid();
} EndProp()
BeginProp(RBool, isBlockOccupied, "Is Block Occupied", "True if the block this signal is observing is currently occupied by a vehicle.") {
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	FIRReturn self->Block.Pin()->IsOccupied();
} EndProp()
BeginProp(RBool, isPathBlock, "Is Path Block", "True if the block this signal is observing is a path-block.") {
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	FIRReturn self->Block.Pin()->IsPathBlock();
} PropSet() {
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	self->Block.Pin()->SetIsPathBlock(Val);
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Returns the blocks validation status.") {
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	FIRReturn (int64)self->Block.Pin()->GetBlockValidation();
} EndProp()
BeginFunc(isOccupiedBy, "Is Occupied By", "Allows you to check if this block is occupied by a given train.") {
	InVal(0, RObject<AFGTrain>, train, "Train", "The train you want to check if it occupies this block")
	OutVal(1, RBool, isOccupied, "Is Occupied", "True if the given train occupies this block.")
	Body()
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	isOccupied = self->Block.Pin()->IsOccupiedBy(train.Get());
} EndFunc()
BeginFunc(getOccupation, "Get Occupation", "Returns a list of trains that currently occupate the block.") {
	OutVal(0, RArray<RTrace<AFGTrain>>, occupation, "Occupation", "A list of trains occupying the block.")
	Body()
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	TArray<FIRAny> Occupation;
	for (TWeakObjectPtr<AFGRailroadVehicle> train : FIRRailroadHelper::FFGRailroadSignalBlock_GetOccupiedBy(*self->Block.Pin())) {
		if (train.IsValid()) Occupation.Add(Ctx.GetTrace() / train.Get());
	}
	occupation = Occupation;
} EndFunc()
BeginFunc(getQueuedReservations, "Get Queued Reservations", "Returns a list of trains that try to reserve this block and wait for approval.") {
	OutVal(0, RArray<RTrace<AFGTrain>>, reservations, "Reservations", "A list of trains that try to reserve this block and wait for approval.")
	Body()
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	TArray<FIRAny> Reservations;
	for (TSharedPtr<FFGRailroadBlockReservation> Reservation : FIRRailroadHelper::FFGRailroadSignalBlock_GetQueuedReservations(*self->Block.Pin())) {
		if (!Reservation.IsValid()) continue;
		AFGTrain* Train = Reservation->Train.Get();
		if (Train) Reservations.Add(Ctx.GetTrace() / Train);
	}
	reservations = Reservations;
} EndFunc()
BeginFunc(getApprovedReservations, "Get Approved Reservations", "Returns a list of trains that are approved by this block.") {
	OutVal(0, RArray<RTrace<AFGTrain>>, reservations, "Reservations", "A list of trains that are approved by this block.")
	Body()
	if (!self->Block.IsValid()) throw FFIRException(TEXT("Signalblock is invalid"));
	TArray<FIRAny> Reservations;
	for (TSharedPtr<FFGRailroadBlockReservation> Reservation : FIRRailroadHelper::FFGRailroadSignalBlock_GetApprovedReservations(*self->Block.Pin())) {
		if (!Reservation.IsValid()) continue;
		AFGTrain* Train = Reservation->Train.Get();
		if (Train) Reservations.Add(Ctx.GetTrace() / Train);
	}
	reservations = Reservations;
} EndFunc()
EndStruct()

BeginStructConstructable(FFIRTargetPoint, "TargetPoint", "Target Point", "Target Point in the waypoint list of a wheeled vehicle.")
BeginProp(RStruct<FVector>, pos, "Pos", "The position of the target point in the world.") {
	FIRReturn self->Pos;
} PropSet() {
	self->Pos = Val;
} EndProp()
BeginProp(RStruct<FRotator>, rot, "Rot", "The rotation of the target point in the world.") {
	FIRReturn self->Rot;
} PropSet() {
	self->Rot = Val;
} EndProp()
BeginProp(RFloat, speed, "Speed", "The speed at which the vehicle should pass the target point.") {
	FIRReturn self->Speed;
} PropSet() {
	self->Speed = Val;
} EndProp()
BeginProp(RFloat, wait, "Wait", "The amount of time which needs to pass till the vehicle will continue to the next target point.") {
	FIRReturn self->Wait;
} PropSet() {
	self->Wait = Val;
} EndProp()
EndStruct()
