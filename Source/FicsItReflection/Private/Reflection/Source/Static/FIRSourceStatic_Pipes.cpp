﻿#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/Source/Static/FIRSourceStaticHooks.h"

#include "FGPipeConnectionComponent.h"
#include "FGPipeSubsystem.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "Buildables/FGPipeHyperStart.h"

class FFIRPipeHelper {
	public:
	static void AFGPipeUpdate(AFGBuildablePipelinePump* Pump) {
		Pump->UpdateFlowLimitOnFluidBox();
	}
};

BeginClass(AFGPipeHyperStart, "PipeHyperStart", "Pipe Hyper Start", "A actor that is a hypertube entrance buildable")
	Hook(UFIRPipeHyperStartHook)
BeginSignal(PlayerEntered, "Player Entered", "Triggers when a players enters into this hypertube entrance.")
	SignalParam(0, RBool, success, "Sucess", "True if the transfer was sucessfull")
EndSignal()
BeginSignal(PlayerExited, "Player Exited", "Triggers when a players leaves through this hypertube entrance.")
EndSignal()
EndClass()

BeginClass(AFGBuildablePipeHyper, "BuildablePipeHyper", "Buildable Pipe Hyper", "A hypertube pipe")
EndClass()

BeginClass(UFGPipeConnectionComponentBase, "PipeConnectionBase", "Pipe Connection Base", "A actor component base that is a connection point to which a pipe for fluid or hyper can get attached to.")
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	FIRReturn self->IsConnected();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected pipe connection component.") {
	OutVal(0, RTrace<UFGPipeConnectionComponentBase>, connected, "Connected", "The connected pipe connection component.")
	Body()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(UFGPipeConnectionComponent, "PipeConnection", "Pipe Connection", "A actor component that is a connection point to which a fluid pipe can get attached to.")
//Hook(UFIRFactoryConnectorHook)
BeginProp(RFloat, fluidBoxContent, "Fluid Box Content", "Returns the amount of fluid this fluid container contains") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, fluidBoxHeight, "Fluid Box Height", "Returns the height of this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->Height;
} EndProp()
BeginProp(RFloat, fluidBoxLaminarHeight, "Fluid Box Laminar Height", "Returns the laminar height of this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->LaminarHeight;
} EndProp()
BeginProp(RFloat, fluidBoxFlowThrough, "Fluid Box Flow Through", "Returns the amount of fluid flowing through this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->FlowThrough;
} EndProp()
BeginProp(RFloat, fluidBoxFlowFill, "Fluid Box Flow Fill", "Returns the fill rate of this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, fluidBoxFlowDrain, "Fluid Box Flow Drain", "Returns the drain rate of this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, fluidBoxFlowLimit, "Fluid Box Flow Limit", "Returns the the maximum flow limit of this fluid container") {
	FIRReturn self->GetFluidIntegrant()->GetFluidBox()->FlowLimit;
} EndProp()
BeginProp(RInt, networkID, "Get Network ID", "Returns the network ID of the pipe network this connection is associated with") {
	FIRReturn (int64)self->GetPipeNetworkID();
} EndProp();
BeginFunc(getFluidDescriptor, "Get Fluid Descriptor", "?") {  /* TODO: Write DOC when figured out exactly what it does */
	OutVal(0, RTrace<UFGItemDescriptor>, fluidDescriptor, "Fluid Descriptor", "?")   /* TODO: Write DOC */
	Body()
	fluidDescriptor = Ctx.GetTrace() / self->GetFluidDescriptor();
} EndFunc()
/*BeginFunc(getFluidIntegrant, "Get Fluid Integrant", "?") {
	OutVal(0, RObject<IFGFluidIntegrantInterface>, fluidIntegrant, "Fluid Descriptor", "?")
    Body()
    fluidIntegrant = Ctx.GetTrace() / self->GetFluidIntegrant();
} EndFunc()*/
BeginFunc(flushPipeNetwork, "Flush Pipe Network", "Flush the associated pipe network") {
    Body()
	auto networkID = self->GetPipeNetworkID();
    auto subsystem = AFGPipeSubsystem::GetPipeSubsystem(self->GetWorld());
	subsystem->FlushPipeNetwork(networkID);
} EndFunc()
EndClass()

BeginClass(AFGBuildablePipeReservoir, "PipeReservoir", "Pipe Reservoir", "The base class for all fluid tanks.")
BeginFunc(flush, "Flush", "Emptys the whole fluid container.") {
	Body()
	AFGPipeSubsystem::Get(self->GetWorld())->FlushIntegrant(self);
} EndFunc()
BeginFunc(getFluidType, "Get Fluid Type", "Returns the type of the fluid.") {
	OutVal(0, RClass<UFGItemDescriptor>, type, "Type", "The type of the fluid the tank contains.")
	Body()
	type = (UClass*)self->GetFluidDescriptor();
} EndFunc()
BeginProp(RFloat, fluidContent, "Fluid Content", "The amount of fluid in the tank.") {
	FIRReturn self->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, maxFluidContent, "Max Fluid Content", "The maximum amount of fluid this tank can hold.") {
	FIRReturn self->GetFluidBox()->MaxContent;
} EndProp()
BeginProp(RFloat, flowFill, "Flow Fill", "The currentl inflow rate of fluid.") {
	FIRReturn self->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, flowDrain, "Float Drain", "The current outflow rate of fluid.") {
	FIRReturn self->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The maximum flow rate of fluid this tank can handle.") {
	FIRReturn self->GetFluidBox()->FlowLimit;
} EndProp()
EndClass()

BeginClass(AFGBuildablePipelinePump, "PipelinePump", "PipelinePump", "A building that can pump fluids to a higher level within a pipeline.")
BeginProp(RFloat, maxHeadlift, "Max Headlift", "The maximum amount of headlift this pump can provide.") {
	FIRReturn self->GetFluidBox()->AddedPressure;
} EndProp()
BeginProp(RFloat, designedHeadlift, "Designed Headlift", "The amomunt of headlift this pump is designed for.") {
	FIRReturn self->GetDesignHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadlift, "Indicator Headlift", "The amount of headlift the indicator shows.") {
	FIRReturn self->GetFluidBox()->ElevationPressureColumn;
} EndProp()
// TODO: Remove/Keep? Keep for now
//BeginProp(RFloat, indicatorHeadliftPct, "Indicator Headlift Percent", "The amount of headlift the indicator shows as percantage from max.") {
//	FIRReturn self->GetIndicatorHeadLiftPct();
//} EndProp()
BeginProp(RFloat, userFlowLimit, "User Flow Limit", "The flow limit of this pump the user can specifiy. Use -1 for no user set limit. (in m^3/s)") {
	FIRReturn self->GetUserFlowLimit();
} PropSet() {
	self->SetUserFlowLimit(Val);
	FFIRPipeHelper::AFGPipeUpdate(self); // @TODO: Not sure if needed. Keep for now
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The overal flow limit of this pump. (in m^3/s)") {
	FIRReturn self->GetFluidBox()->FlowLimit;
} EndProp()
// TODO: Remove/Keep? Keep for now
//BeginProp(RFloat, flowLimitPct, "Flow Limit Pct", "The overal flow limit of this pump. (in percent)") {
//	float FlowLimit = self->GetFluidBox()->FlowLimit;
//	float DefaultFlowLimit = self->GetDefaultFlowLimit();
//	if(FlowLimit < 0) {
//		FlowLimit = DefaultFlowLimit;
//	}
//	FIRReturn (FlowLimit / DefaultFlowLimit);
//} EndProp()
BeginProp(RFloat, defaultFlowLimit, "Default Flow Limit", "Get the set maximum flow rate through this pump. [m3/s]") {
	FIRReturn self->GetDefaultFlowLimit();
} EndProp()
BeginProp(RFloat, flow, "Flow", "The current flow amount. (in m^3/s)") {
	FIRReturn abs(self->GetFluidBox()->FlowThrough);
} EndProp()
// TODO: Remove/Keep? Keep for now
//BeginProp(RFloat, flowPct, "Flow Pct", "The current flow amount. (in percent)") {
//	float FlowLimit = self->GetFluidBox()->FlowLimit;
//	float Return;
//	if(FlowLimit == 0) {
//		Return = 0;
//		goto End;		
//	}
//	if(FlowLimit < 0) {
//		FlowLimit = self->GetDefaultFlowLimit();
//	}
//	Return = (abs(self->GetFluidBox()->FlowThrough) / FlowLimit)
//	End:
//	FIRReturn Return;
//} EndProp()
EndClass()
