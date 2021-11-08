#include "FIVSNode_Proxy.h"

#include "FicsItNetworks/Reflection/FINReflection.h"

void UFIVSNode_Proxy::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add(
		{
			UFIVSNode_Proxy::StaticClass(),
			FText::FromString(TEXT("Proxy")),
			FText::FromString(TEXT("General")),
			FText::FromString(TEXT("Proxy")),
			{
				FIVS_PIN_EXEC_INPUT,
				{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIN_STR)},
				FIVS_PIN_EXEC_OUTPUT,
				{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass()))}
			}
		}
	);
}

void UFIVSNode_Proxy::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	AddrIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Address"), FText::FromString("Address"), FFIVSPinDataType(FIN_STR));
	CompOut = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Component"), FText::FromString("Component"), FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())));
}

UFIVSPin* UFIVSNode_Proxy::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FString Addr = Context.GetValue(AddrIn)->GetString();
	FGuid Guid;
	if (!FGuid::Parse(Addr, Guid)) {
		Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Address not valid!")));
		return nullptr;
	}
	FFINNetworkTrace Component = Context.GetKernelContext()->GetNetwork()->GetComponentByID(Guid);
	if (!Component.IsValid()) {
		Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Component not found!")));
	}
	Context.SetValue(CompOut, Component);
	return ExecOut;
}