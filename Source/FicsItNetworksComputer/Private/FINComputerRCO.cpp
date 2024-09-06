#include "FINComputerRCO.h"

#include "FINComputerCase.h"
#include "FINComputerEEPROMDesc.h"
#include "FINNetworkComponent.h"
#include "ComputerModules/FINComputerDriveHolder.h"
#include "ComputerModules/PCI/FINComputerGPUT1.h"
#include "ComputerModules/PCI/FINComputerGPUT2.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "Net/UnrealNetwork.h"

void UFINComputerRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINComputerRCO, bDummy);
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
	default: break;
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
	default: break;
	}
}

bool UFINComputerRCO::GPUKeyEvent_Validate(AFINComputerGPUT1* GPU, int type, int64 c, int64 code, int btn) {
	return true;
}

void UFINComputerRCO::GPUKeyCharEvent_Implementation(AFINComputerGPUT1* GPU, const FString& c, int btn) {
	GPU->netSig_OnKeyChar(c, btn);
}

bool UFINComputerRCO::GPUKeyCharEvent_Validate(AFINComputerGPUT1* GPU, const FString& c, int btn) {
	return true;
}

void UFINComputerRCO::CreateEEPROMState_Implementation(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.GetItemClass())) return;
	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.GetItemClass()->GetDefaultObject());
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

void UFINComputerRCO::CopyDataItem_Implementation(UFGInventoryComponent* InProviderInc, int InProviderIdx, UFGInventoryComponent* InFromInv, int InFromIdx, UFGInventoryComponent* InToInv, int InToIdx) {
	FInventoryStack Provider;
	if (!InProviderInc->GetStackFromIndex(InProviderIdx, Provider)) return;
	FInventoryStack From;
	if (!InFromInv->GetStackFromIndex(InFromIdx, From)) return;
	if (!InToInv->IsIndexEmpty(InToIdx)) return;
	if (!Provider.Item.GetItemClass() || !From.Item.GetItemClass()) return;
	UObject* Descriptor = const_cast<UObject*>(GetDefault<UObject>(Provider.Item.GetItemClass()));
	if (!Descriptor->Implements<UFINCopyableItemInterface>()) return;
	bool bDone = IFINCopyableItemInterface::Execute_CopyData(Descriptor, InProviderInc, Provider.Item, From.Item, From.Item);
	if (bDone) {
		InFromInv->RemoveAllFromIndex(InFromIdx);
		InToInv->AddStackToIndex(InToIdx, From);
	}
}

bool UFINComputerRCO::CopyDataItem_Validate(UFGInventoryComponent* InProviderInc, int InProviderIdx, UFGInventoryComponent* InFromInv, int InFromIdx, UFGInventoryComponent* InToInv, int InToIdx) {
	return true;
}

void UFINComputerRCO::GPUT2MouseEvent_Implementation(AFINComputerGPUT2* GPU, int Type, FVector2D Position, int Modifiers) {
	switch (Type) {
	case 0:
		GPU->netSig_OnMouseDown(Position, Modifiers);
		break;
	case 1:
		GPU->netSig_OnMouseUp(Position, Modifiers);
		break;
	case 2:
		GPU->netSig_OnMouseMove(Position, Modifiers);
		break;
	case 3:
		GPU->netSig_OnMouseEnter(Position, Modifiers);
		break;
	case 4:
		GPU->netSig_OnMouseLeave(Position, Modifiers);
		break;
	default: break;
	}
}

bool UFINComputerRCO::GPUT2MouseEvent_Validate(AFINComputerGPUT2* GPU, int Type, FVector2D Position, int Modifiers) {
	return true;
}

void UFINComputerRCO::GPUT2KeyEvent_Implementation(AFINComputerGPUT2* GPU, int Type, int64 C, int64 Code, int Modifiers) {
	switch (Type) {
	case 0:
		GPU->netSig_OnKeyDown(C, Code, Modifiers);
		break;
	case 1:
		GPU->netSig_OnKeyUp(C, Code, Modifiers);
		break;
	default: break;
	}
}

bool UFINComputerRCO::GPUT2KeyEvent_Validate(AFINComputerGPUT2* GPU, int Type, int64 C, int64 Code, int Modifiers) {
	return true;
}

void UFINComputerRCO::GPUT2KeyCharEvent_Implementation(AFINComputerGPUT2* GPU, const FString& C, int Modifiers) {
	GPU->netSig_OnKeyChar(C, Modifiers);
}

bool UFINComputerRCO::GPUT2KeyCharEvent_Validate(AFINComputerGPUT2* GPU, const FString& C, int Modifiers) {
	return true;
}

void UFINComputerRCO::GPUT2MouseWheelEvent_Implementation(AFINComputerGPUT2* GPU, FVector2D Position, float Delta, int Modifiers) {
	GPU->netSig_OnMouseWheel(Position, Delta, Modifiers);
}

void UFINComputerRCO::SetLabel_Implementation(UObject* Container, const FString& Label) {
	if (IsValid(Container) && Container->Implements<UFINLabelContainerInterface>()) {
		IFINLabelContainerInterface::Execute_SetLabel(Container, Label);
	}
}
