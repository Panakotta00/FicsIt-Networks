#include "FIVSNode_OnTick.h"

TArray<FFIVSNodeAction> UFIVSNode_OnTick::GetNodeActions() const {
	return {
			{
				UFIVSNode_OnTick::StaticClass(),
				FText::FromString(TEXT("On Tick")),
				FText::FromString(TEXT("General|Events")),
				FText::FromString(TEXT("On Tick")),
				{
					FIVS_PIN_EXEC_OUTPUT
				}
			}
	};
}

void UFIVSNode_OnTick::InitPins() {
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Run"));
}

UFIVSPin* UFIVSNode_OnTick::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return ExecOut;
}