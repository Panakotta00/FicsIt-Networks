#include "Script/Library/FIVSNode_OnStart.h"

#include "Kernel/FIVSRuntimeContext.h"

UFIVSNode_OnStart::UFIVSNode_OnStart() {
	DisplayName = FText::FromString(TEXT("Event Start"));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Run"), FText::FromString("Run"));
}

void UFIVSNode_OnStart::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add({
		UFIVSNode_OnStart::StaticClass(),
		FText::FromString(TEXT("On Start")),
		FText::FromString(TEXT("General|Events")),
		FText::FromString(TEXT("On Start")),
		{
			FIVS_PIN_EXEC_OUTPUT
		}
	});
}

void UFIVSNode_OnStart::CompileNodeToLua(FFIVSLuaCompilerContext& Context) const {
	Context.AddPlain(TEXT(R"(
future.addTask(async(function()
)"));
	Context.EnterNewSection(1);
	Context.ContinueCurrentSection(ExecOut);
	Context.LeaveSection(-1);
	Context.AddPlain(TEXT(R"(
end))
)"));
}
