#include "Script/Library/FIVSNode_OnTick.h"

UFIVSNode_OnTick::UFIVSNode_OnTick() {
	DisplayName = FText::FromString(TEXT("Event Tick"));

	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Run"), FText::FromString("Run"));
}

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

TArray<UFIVSPin*> UFIVSNode_OnTick::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return {ExecOut};
}
