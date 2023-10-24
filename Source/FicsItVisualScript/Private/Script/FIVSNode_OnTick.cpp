#include "Script/FIVSNode_OnTick.h"

void UFIVSNode_OnTick::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add(
{
			UFIVSNode_OnTick::StaticClass(),
			FText::FromString(TEXT("On Tick")),
			FText::FromString(TEXT("General|Events")),
			FText::FromString(TEXT("On Tick")),
			{
				FIVS_PIN_EXEC_OUTPUT
			}
		}
	);
}

void UFIVSNode_OnTick::InitPins() {
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Run"), FText::FromString("Run"));
}

UFIVSPin* UFIVSNode_OnTick::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return ExecOut;
}