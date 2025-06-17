#include "FINComponentUtility.h"

#include "FGGameRulesSubsystem.h"
#include "FGInventoryComponent.h"
#include "FINItemStateFileSystem.h"
#include "ReflectionHelper.h"
#include "Widget.h"
#include "HAL/PlatformApplicationMisc.h"

struct FFINItemStateFileSystem;

void UFINComponentUtility::ClipboardCopy(FString str) {
	FPlatformApplicationMisc::ClipboardCopy(*str);
}

bool UFINComponentUtility::GetNoUnlockCost(UObject* WorldContext) {
	return AFGGameRulesSubsystem::Get(WorldContext)->GetNoUnlockCost();
}

FGuid UFINComponentUtility::GetFileSystemStateFromSlotWidget(UWidget* InSlot) {
	struct {
		FInventoryStack Stack;
	} Params;
	FReflectionHelper::CallScriptFunction(InSlot, TEXT("GetStack"), &Params);
	const FFINItemStateFileSystem* State = Params.Stack.Item.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
	if (State) {
		return State->ID;
	}
	return FGuid();
}

FString UFINComponentUtility::GetLabelFromSlot(UWidget* InSlot, bool& CanGetLabel) {
	struct {
		FInventoryStack Stack;
	} Params;
	FReflectionHelper::CallScriptFunction(InSlot, TEXT("GetStack"), &Params);

	if (!Params.Stack.Item.HasState()) return {};

	const FFINLabelContainerInterface* container = FFINStructInterfaces::Get().GetInterface<FFINLabelContainerInterface>(Params.Stack.Item.GetItemState());
	if (container) {
		CanGetLabel = true;
		return container->GetLabel();
	}
	CanGetLabel = false;
	return {};
}

