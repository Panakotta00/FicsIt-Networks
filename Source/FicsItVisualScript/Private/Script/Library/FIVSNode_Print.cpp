#include "Script/Library/FIVSNode_Print.h"

#include "FicsItKernel/Logging.h"

void UFIVSNode_Print::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add(
		{
			UFIVSNode_Print::StaticClass(),
			FText::FromString(TEXT("Print")),
			FText::FromString(TEXT("General")),
			FText::FromString(TEXT("Print")),
			{
				FIVS_PIN_EXEC_INPUT,
				{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIN_STR)},
				FIVS_PIN_EXEC_OUTPUT,
			}
		}
	);
}

void UFIVSNode_Print::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	MessageIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Message"), FText::FromString("Message"), FFIVSPinDataType(FIN_STR));
}

TArray<UFIVSPin*> UFIVSNode_Print::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{MessageIn};
}

UFIVSPin* UFIVSNode_Print::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FString Message = Context.GetValue(MessageIn)->GetString();
	Context.GetKernelContext()->GetLog()->PushLogEntry(FIN_Log_Verbosity_Info, Message);
	return ExecOut;
}
