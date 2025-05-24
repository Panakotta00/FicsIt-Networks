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

void UFIVSNode_OnTick::CompileNodeToLua(FFIVSLuaCompilerContext& Context) {
	/*Context.AddPlain(TEXT(R"(
future.addTask(function()
	while true do
		{EnterNewSection}
		{0}
		{LeaveNewSection}
		coroutine.yield()
	end
end
)"), ExecOut);*/

	// Or without templating

	Context.AddPlain(TEXT(R"(
future.addTask(async(function()
	while true do
)"));
	Context.EnterNewSection(2);
	Context.ContinueCurrentSection(ExecOut);
	Context.LeaveSection(-2);
	Context.AddPlain(TEXT(R"(
		coroutine.yield()
	end
end))
)"));
}
