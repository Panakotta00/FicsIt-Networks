#include "stdafx.h"

#include <assets/AssetFunctions.h>

#include "Equip_FileSystem.h"

using namespace SML;
using namespace SML::Objects;

void AEquip_FileSystem::construct() {
	//static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	//if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");
	((bool(AEquip_FileSystem::**)() const)Vtable)[0xCA] = &AEquip_FileSystem::shouldSaveState;
	auto self = (Objects::UObject*)this;

	RootComponent = self->createDefaultSubobjectSDK<SDK::USceneComponent>(L"RootComponent");

	filesystem = self->createDefaultSubobject<UFileSystem>(L"FileSystem");
}

void AEquip_FileSystem::destruct() {
	
}

bool AEquip_FileSystem::shouldSaveState() const {
	return true;
}

struct FReferenceControllerBase {
	void* vfptr;
	int SharedReferenceCount;
	int WeakReferenceCount;
};

struct FSharedReferencer {
	FReferenceControllerBase* ReferenceController;
};

struct FSharedInventoryStatePtr {
	SDK::AActor* ActorPtr;
	FSharedReferencer referencer;
};

struct FInventoryItem {
	class UClass* ItemClass;
	struct FSharedInventoryStatePtr ItemState;
};

void AEquip_FileSystem::moveSelfToItem(SML::Objects::FFrame & stack, void * item) {
	static void(*constructFInventoryStack)(SDK::FInventoryStack*) = nullptr;
	if (!constructFInventoryStack) constructFInventoryStack = (void(*)(SDK::FInventoryStack*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FInventoryStack::FInventoryStack");
	static void(*setStateOnIndex)(SDK::UFGInventoryComponent*, int, SDK::FSharedInventoryStatePtr*) = nullptr;
	if (!setStateOnIndex) setStateOnIndex = (void(*)(SDK::UFGInventoryComponent*, int, SDK::FSharedInventoryStatePtr*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGInventoryComponent::SetStateOnIndex");
	static void(*constructFSharedInventoryStatePtr)(SDK::FSharedInventoryStatePtr*, void*) = nullptr;
	if (!constructFSharedInventoryStatePtr) constructFSharedInventoryStatePtr = (void(*)(SDK::FSharedInventoryStatePtr*, void*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FSharedInventoryStatePtr::FSharedInventoryStatePtr");
	static void(*getStackFromIndex)(SDK::UFGInventoryComponent*, int, SDK::FInventoryStack*) = nullptr;
	if (!getStackFromIndex) getStackFromIndex = (void(*)(SDK::UFGInventoryComponent*, int, SDK::FInventoryStack*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGInventoryComponent::GetStackFromIndex");
	static void(*removeFromIndex)(SDK::UFGInventoryComponent*, int, int) = nullptr;
	if (!removeFromIndex) removeFromIndex = (void(*)(SDK::UFGInventoryComponent*, int, int)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGInventoryComponent::RemoveFromIndex");

	stack.code += !!stack.code;
	
	auto c = Mod::Functions::getPlayerCharacter();
	auto inv = c->GetEquipmentSlot(SDK::EEquipmentSlot::ES_ARMS);
	auto statePtr = SDK::FSharedInventoryStatePtr();
	constructFSharedInventoryStatePtr(&statePtr, this);
	setStateOnIndex(inv, inv->GetActiveIndex(), &statePtr);
	SDK::FInventoryStack s = SDK::FInventoryStack();
	constructFInventoryStack(&s);
	getStackFromIndex(inv, inv->GetActiveIndex(), &s);
	*((SDK::FInventoryItem*)item) = s.Item;
	Utility::error("LifeSpan2: ", GetLifeSpan());
	removeFromIndex(inv, inv->GetActiveIndex(), 1);
	Utility::error("LifeSpan3: ", GetLifeSpan());
}
