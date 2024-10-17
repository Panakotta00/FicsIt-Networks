#include "Script/Library/FIVSNode_Proxy.h"

#include "FicsItReflection.h"
#include "FINNetworkUtils.h"
#include "NetworkController.h"
#include "Kernel/FIVSRuntimeContext.h"

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
	FFIRTrace Component = Context.GetKernelContext()->GetNetwork()->GetComponentByID(Guid);
	if (!Component.IsValid()) {
		Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Component not found!")));
		return;
	}
	FFIRTrace instance = UFINNetworkUtils::RedirectIfPossible(Component);
	Context.SetValue(CompOut, instance);
	Context.Push_ExecPin(ExecOut);
}

UFIVSNode_Proxy::UFIVSNode_Proxy() {
	DisplayName = FText::FromString(TEXT("Proxy"));

	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	AddrIn = CreateDefaultPin(FIVS_PIN_DATA_INPUT, TEXT("Address"), FText::FromString("Address"), FFIVSPinDataType(FIR_STR));
	CompOut = CreateDefaultPin(FIVS_PIN_DATA_OUTPUT, TEXT("Component"), FText::FromString("Component"), FFIVSPinDataType(FIR_TRACE, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass())));
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
				{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIR_STR)},
				FIVS_PIN_EXEC_OUTPUT,
				{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIR_TRACE, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass()))}
			}
		}
	);
}
