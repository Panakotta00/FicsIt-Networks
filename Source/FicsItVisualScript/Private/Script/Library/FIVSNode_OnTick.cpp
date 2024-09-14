#include "Script/Library/FIVSNode_OnTick.h"

#include "Kernel/FIVSRuntimeContext.h"

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

void FFIVSNodeStatement_OnTick::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	FFIVSNodeStatement::PreExecPin(Context, ExecPin);
}

void FFIVSNodeStatement_OnTick::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_ExecPin(ExecOut);
}
