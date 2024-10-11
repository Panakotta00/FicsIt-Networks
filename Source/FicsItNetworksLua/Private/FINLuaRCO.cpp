#include "FINLuaRCO.h"

#include "FGInventoryComponent.h"
#include "FINComputerEEPROMDesc.h"
#include "FINStateEEPROMLua.h"
#include "Net/UnrealNetwork.h"

void UFINLuaRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINLuaRCO, bDummy);
}

void UFINLuaRCO::SetLuaEEPROMCode_Implementation(UFGInventoryComponent* Inventory, int32 Index, const FString& NewCode) {
	FInventoryStack stack;
	if (!Inventory->GetStackFromIndex(Index, stack)) return;
	UFINComputerEEPROMDesc::CreateEEPROMStateInItem(stack.Item);
	if (const FFINStateEEPROMLua* luaState = stack.Item.GetItemState().GetValuePtr<FFINStateEEPROMLua>()) {
		FFINStateEEPROMLua state = *luaState;
		state.Code = NewCode;
		Inventory->SetStateOnIndex(Index, FFGDynamicStruct(state));
	}
}
