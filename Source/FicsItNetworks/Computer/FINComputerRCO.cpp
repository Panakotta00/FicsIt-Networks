#include "FINComputerRCO.h"
#include "FINComputerEEPROMDesc.h"
#include "FINComputerGPUT1.h"

void UFINComputerRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINComputerRCO, bDummy);
}

void UFINComputerRCO::SetLuaEEPROMCode_Implementation(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	LuaEEPROMState->SetCode(NewCode);
}

bool UFINComputerRCO::SetLuaEEPROMCode_Validate(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	return true;
}

void UFINComputerRCO::SetCaseLastTab_Implementation(AFINComputerCase* Case, int LastTab) {
	Case->LastTabIndex = LastTab;
	Case->ForceNetUpdate();
}

bool UFINComputerRCO::SetCaseLastTab_Validate(AFINComputerCase* Case, int LastTab) {
	return true;
}

void UFINComputerRCO::SetDriveHolderLocked_Implementation(AFINComputerDriveHolder* Holder, bool bLocked) {
	Holder->SetLocked(bLocked);
}

bool UFINComputerRCO::SetDriveHolderLocked_Validate(AFINComputerDriveHolder* Holder, bool bLocked) {
	return true;
}

void UFINComputerRCO::ToggleCase_Implementation(AFINComputerCase* Case) {
	Case->Toggle();
}

bool UFINComputerRCO::ToggleCase_Validate(AFINComputerCase* Case) {
	return true;
}

void UFINComputerRCO::SetNick_Implementation(UObject* Component, const FString& Nick) {
	if (Component->Implements<UFINNetworkComponent>() && IsValid(Component)) IFINNetworkComponent::Execute_SetNick(Component, Nick);
}

bool UFINComputerRCO::SetNick_Validate(UObject* Component, const FString& Nick) {
	return true;
}

void UFINComputerRCO::GPUMouseEvent_Implementation(AFINComputerGPUT1* GPU, int type, int x, int y, int btn) {
	switch (type) {
	case 0:
		GPU->netSig_OnMouseDown(x, y, btn);
		break;
	case 1:
		GPU->netSig_OnMouseUp(x, y, btn);
		break;
	case 2:
		GPU->netSig_OnMouseMove(x, y, btn);
		break;
	}
}

bool UFINComputerRCO::GPUMouseEvent_Validate(AFINComputerGPUT1* GPU, int type, int x, int y, int btn) {
	return true;
}

void UFINComputerRCO::GPUKeyEvent_Implementation(AFINComputerGPUT1* GPU, int type, int64 c, int64 code, int btn) {
	switch (type) {
	case 0:
		GPU->netSig_OnKeyDown(c, code, btn);
		break;
	case 1:
		GPU->netSig_OnKeyUp(c, code, btn);
		break;
	}
}

bool UFINComputerRCO::GPUKeyEvent_Validate(AFINComputerGPUT1* GPU, int type, int64 c, int64 code, int btn) {
	return true;
}

void UFINComputerRCO::CreateEEPROMState_Implementation(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.ItemClass)) return;
	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.ItemClass->GetDefaultObject());
	if (!IsValid(desc)) return;
	UClass* clazz = desc->EEPROMStateClass;
	
	FVector loc = FVector::ZeroVector;
	FRotator rot = FRotator::ZeroRotator;
	FActorSpawnParameters params;
	params.bNoFail = true;
	AFINStateEEPROM* eeprom = Inv->GetWorld()->SpawnActor<AFINStateEEPROM>(clazz, loc, rot, params);
	if (!IsValid(eeprom)) return;
	Inv->SetStateOnIndex(SlotIdx, FSharedInventoryStatePtr::MakeShared((AActor*)eeprom));
}

bool UFINComputerRCO::CreateEEPROMState_Validate(UFGInventoryComponent* Inv, int SlotIdx) {
	return true;
}
