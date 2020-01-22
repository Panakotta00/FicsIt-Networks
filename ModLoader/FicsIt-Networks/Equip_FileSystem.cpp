#include "stdafx.h"

#include <mod/ModFunctions.h>
#include <assets/AssetFunctions.h>
#include <assets/FObjectSpawnParameters.h>
#include <game/Global.h>

#include <util/Objects/FVector.h>

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
	new (&stack) SDK::FInventoryStack();
	new (&save) IEquipFileSystemSaveInterface();

	static void(*constructFInventoryStack)(SDK::FInventoryStack*) = nullptr;
	if (!constructFInventoryStack) constructFInventoryStack = (void(*)(SDK::FInventoryStack*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FInventoryStack::FInventoryStack");
	constructFInventoryStack(&stack);
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

void AEquip_FileSystem::createState(SML::Objects::FFrame & stack, void * state) {
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

	int capacity = 0;
	SDK::UFGInventoryComponent* inv = nullptr;
	int slot = 0;

	stack.stepCompIn(&capacity);
	stack.stepCompIn(&inv);
	stack.stepCompIn(&slot);

	stack.code += !!stack.code;

	FActorSpawnParameters spawnParams;
	SDK::FVector loc{0,0,0};
	SDK::FRotator rot{0,0,0};
	auto efs = (AEquip_FileSystem*)::call<&SML::Objects::UWorld::SpawnActor>((SML::Objects::UWorld*)*SDK::UWorld::GWorld, (SDK::UClass*)staticClass(), &loc, &rot, (FActorSpawnParameters*)&spawnParams);
	efs->filesystem->capacity = capacity;

	Utility::error("LifeSpan1: ", efs->GetLifeSpan());
	auto statePtr = SDK::FSharedInventoryStatePtr();
	constructFSharedInventoryStatePtr(&statePtr, efs);
	Utility::error("LifeSpan2: ", efs->GetLifeSpan());
	setStateOnIndex(inv, slot, &statePtr);
	Utility::error("LifeSpan3: ", efs->GetLifeSpan());

	*((AEquip_FileSystem**)state) = efs;
}

SML::Objects::UClass * AEquip_FileSystem::staticClass() {
	return Paks::ClassBuilder<AEquip_FileSystem>::staticClass();
}

bool IEquipFileSystemSaveInterface::NeedTransform() {
	return false;
}

bool IEquipFileSystemSaveInterface::ShouldSave() {
	return true;
}

void IEquipFileSystemSaveInterface::gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*) {}

void IEquipFileSystemSaveInterface::postLoad(int, int) {}

void IEquipFileSystemSaveInterface::preLoad(int, int) {}

void IEquipFileSystemSaveInterface::postSave(int, int) {}

void IEquipFileSystemSaveInterface::preSave(int, int) {}

SML::Objects::UObject * IEquipFileSystemSaveInterface::_getUObject() const {
	return nullptr;
}
