#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/Source/Static/FIRSourceStaticHooks.h"

#include "FGPipeConnectionComponent.h"
#include "FGPipeSubsystem.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "Buildables/FGPipeHyperStart.h"

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
	Return self->IsConnected();
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
	Return self->GetFluidIntegrant()->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, fluidBoxHeight, "Fluid Box Height", "Returns the height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->Height;
} EndProp()
BeginProp(RFloat, fluidBoxLaminarHeight, "Fluid Box Laminar Height", "Returns the laminar height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->LaminarHeight;
} EndProp()
BeginProp(RFloat, fluidBoxFlowThrough, "Fluid Box Flow Through", "Returns the amount of fluid flowing through this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowThrough;
} EndProp()
BeginProp(RFloat, fluidBoxFlowFill, "Fluid Box Flow Fill", "Returns the fill rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, fluidBoxFlowDrain, "Fluid Box Flow Drain", "Returns the drain rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, fluidBoxFlowLimit, "Fluid Box Flow Limit", "Returns the the maximum flow limit of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowLimit;
} EndProp()
BeginProp(RInt, networkID, "Get Network ID", "Returns the network ID of the pipe network this connection is associated with") {
	Return (int64)self->GetPipeNetworkID();
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
	Return self->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, maxFluidContent, "Max Fluid Content", "The maximum amount of fluid this tank can hold.") {
	Return self->GetFluidBox()->MaxContent;
} EndProp()
BeginProp(RFloat, flowFill, "Flow Fill", "The currentl inflow rate of fluid.") {
	Return self->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, flowDrain, "Float Drain", "The current outflow rate of fluid.") {
	Return self->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The maximum flow rate of fluid this tank can handle.") {
	Return self->GetFluidBox()->FlowLimit;
} EndProp()
EndClass()

BeginClass(AFGBuildablePipelinePump, "PipelinePump", "PipelinePump", "A building that can pump fluids to a higher level within a pipeline.")
BeginProp(RFloat, maxHeadlift, "Max Headlift", "The maximum amount of headlift this pump can provide.") {
	Return self->GetMaxHeadLift();
} EndProp()
BeginProp(RFloat, designedHeadlift, "Designed Headlift", "The amomunt of headlift this pump is designed for.") {
	Return self->GetDesignHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadlift, "Indicator Headlift", "The amount of headlift the indicator shows.") {
	Return self->GetIndicatorHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadliftPct, "Indicator Headlift Percent", "The amount of headlift the indicator shows as percantage from max.") {
	Return self->GetIndicatorHeadLiftPct();
} EndProp()
BeginProp(RFloat, userFlowLimit, "User Flow Limit", "The flow limit of this pump the user can specifiy. Use -1 for now user set limit. (in m^3/s)") {
	Return self->GetUserFlowLimit();
} PropSet() {
	self->SetUserFlowLimit(Val);
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The overal flow limit of this pump. (in m^3/s)") {
	Return self->GetFlowLimit();
} EndProp()
BeginProp(RFloat, flowLimitPct, "Flow Limit Pct", "The overal flow limit of this pump. (in percent)") {
	Return self->GetFlowLimitPct();
} EndProp()
BeginProp(RFloat, flow, "Flow", "The current flow amount. (in m^3/s)") {
	Return self->GetIndicatorFlow();
} EndProp()
BeginProp(RFloat, flowPct, "Float Pct", "The current flow amount. (in percent)") {
	Return self->GetIndicatorFlowPct();
} EndProp()
EndClass()
