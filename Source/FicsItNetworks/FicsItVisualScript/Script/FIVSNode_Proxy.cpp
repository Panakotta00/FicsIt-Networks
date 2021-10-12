#include "FIVSNode_Proxy.h"

TArray<FFIVSNodeAction> UFIVSNode_Proxy::GetNodeActions() const {
	return {
			{
				UFIVSNode_Proxy::StaticClass(),
				FText::FromString(TEXT("Proxy")),
				FText::FromString(TEXT("General")),
				FText::FromString(TEXT("Proxy")),
				{
					FIVS_PIN_EXEC_INPUT,
					{FIVS_PIN_DATA_INPUT, FIN_STR},
					FIVS_PIN_EXEC_OUTPUT,
					{FIVS_PIN_DATA_OUTPUT, {FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())}}
				}
			}
	};
}

void UFIVSNode_Proxy::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
	AddrIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Address"), FIN_STR);
	CompOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Component"), {FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())});
}

UFIVSPin* UFIVSNode_Proxy::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FString Addr = Context.GetValue(AddrIn).GetString();
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