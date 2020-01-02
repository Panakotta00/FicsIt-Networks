#include "stdafx.h"

#include <assets/AssetFunctions.h>

#include "Equip_FileSystem.h"

using namespace SML;
using namespace SML::Objects;

void(AEquip_FileSystem::* oldDestroyed)() = nullptr;

void AEquip_FileSystem::construct() {
	//static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	//if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");

	((bool(AEquip_FileSystem::**)() const)Vtable)[0xCA] = &AEquip_FileSystem::shouldSaveState;
	oldDestroyed = ((void(AEquip_FileSystem::**)())Vtable)[0xA1];
	((void(AEquip_FileSystem::**)())Vtable)[0xA1] = &AEquip_FileSystem::destroyed;
	auto self = (Objects::UObject*)this;

	RootComponent = self->createDefaultSubobjectSDK<SDK::USceneComponent>(L"RootComponent");

	filesystem = self->createDefaultSubobject<UFileSystem>(L"FileSystem");
	
	Utility::warning("construct! ", this->GetName().c_str());
}

void AEquip_FileSystem::destruct() {
	Utility::warning("destruct! ", this->GetName().c_str());
}

bool AEquip_FileSystem::shouldSaveState() const {
	return true;
}

void AEquip_FileSystem::destroyed() {
	Utility::warning("meep!");
	(this->*(oldDestroyed))();
}

void AEquip_FileSystem::moveSelfToItem(SML::Objects::FFrame & stack, void * item) {
	static void(*setStateOnIndex)(SDK::UFGInventoryComponent*, int, SDK::FSharedInventoryStatePtr*) = nullptr;
	if (!setStateOnIndex) setStateOnIndex = (void(*)(SDK::UFGInventoryComponent*, int, SDK::FSharedInventoryStatePtr*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGInventoryComponent::SetStateOnIndex");
	static void(*constructFSharedInventoryStatePtr)(SDK::FSharedInventoryStatePtr*, void*) = nullptr;
	if (!constructFSharedInventoryStatePtr) constructFSharedInventoryStatePtr = (void(*)(SDK::FSharedInventoryStatePtr*, void*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FSharedInventoryStatePtr::FSharedInventoryStatePtr");
	
	stack.code += !!stack.code;
	
	auto c = Mod::Functions::getPlayerCharacter();
	auto inv = c->GetEquipmentSlot(SDK::EEquipmentSlot::ES_ARMS);
	auto statePtr = new SDK::FSharedInventoryStatePtr();
	constructFSharedInventoryStatePtr(statePtr, this);
	setStateOnIndex(inv, inv->GetActiveIndex(), statePtr);
	SDK::FInventoryStack s = SDK::FInventoryStack();
	inv->GetStackFromIndex(inv->GetActiveIndex(), &s);
	*((SDK::FInventoryItem*)item) = s.Item;
	inv->RemoveFromIndex(inv->GetActiveIndex(), 1);
}
