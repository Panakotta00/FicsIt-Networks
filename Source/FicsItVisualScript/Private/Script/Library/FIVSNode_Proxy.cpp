#include "Script/Library/FIVSNode_Proxy.h"

#include "Kernel/FIVSRuntimeContext.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINReflection.h"

void FFIVSNodeStatement_Proxy::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(AddrIn);
}

void FFIVSNodeStatement_Proxy::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	FString Addr = Context.TryGetRValue(AddrIn)->GetString();
	FGuid Guid;
	if (!FGuid::Parse(Addr, Guid)) {
		Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Address not valid!")));
		return;
	}
	FFINNetworkTrace Component = Context.GetKernelContext()->GetNetwork()->GetComponentByID(Guid);
	if (!Component.IsValid()) {
		Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Component not found!")));
		return;
	}
	FFINNetworkTrace instance = UFINNetworkUtils::RedirectIfPossible(Component);
	Context.SetValue(CompOut, instance);
	Context.Push_ExecPin(ExecOut);
}

UFIVSNode_Proxy::UFIVSNode_Proxy() {
	DisplayName = FText::FromString(TEXT("Proxy"));

	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	AddrIn = CreateDefaultPin(FIVS_PIN_DATA_INPUT, TEXT("Address"), FText::FromString("Address"), FFIVSPinDataType(FIN_STR));
	CompOut = CreateDefaultPin(FIVS_PIN_DATA_OUTPUT, TEXT("Component"), FText::FromString("Component"), FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())));
}

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
