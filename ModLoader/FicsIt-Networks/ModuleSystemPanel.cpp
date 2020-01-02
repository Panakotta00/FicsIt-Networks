#include "stdafx.h"
#include "ModuleSystemPanel.h"

#include <util/Objects/FVector.h>

using namespace SML;
using namespace SML::Objects;

static void(UModuleSystemPanel::* postLoad_f)() = nullptr;
static void(UModuleSystemPanel::* beginPlay_f)() = nullptr;
static void(UModuleSystemPanel::* endPlay_f)(SDK::EEndPlayReason) = nullptr;

void UModuleSystemPanel::construct() {
	// actor vtable hook
	if (!postLoad_f) {
		auto& f = ((void(UModuleSystemPanel::**)())this->Vtable)[0xE];
		postLoad_f = f;
		f = &UModuleSystemPanel::postLoad;
	}
	if (!beginPlay_f) {
		auto& f = ((void(UModuleSystemPanel::**)())this->Vtable)[0x5F];
		beginPlay_f = f;
		f = &UModuleSystemPanel::beginPlay;
	}
	if (!endPlay_f) {
		auto& f = ((void(UModuleSystemPanel::**)(SDK::EEndPlayReason))this->Vtable)[0x60];
		endPlay_f = f;
		f = &UModuleSystemPanel::endPlay;
	}
	panelWidth = panelHeight = 1;
	new (&allowedModules) TArray<UClass*>();
}

void UModuleSystemPanel::destruct() {
	
}

void UModuleSystemPanel::endPlay(SDK::EEndPlayReason reason) {
	for (int i = 0; i < panelWidth; ++i) delete[] grid[i];
	delete[] grid;
}

SDK::AActor * UModuleSystemPanel::getModule(int x, int y) {
	return  (x >= 0 && x < panelHeight && y >= 0 && y < panelWidth) ? grid[x][y] : nullptr;;
}

void UModuleSystemPanel::postLoad() {
	grid = new SDK::AActor**[panelWidth]();
	for (int i = 0; i < panelWidth; ++i) {
		grid[i] = new SDK::AActor*[panelHeight];
		std::memset(grid[i], 0, panelWidth * sizeof(void*));
	}
}

void UModuleSystemPanel::beginPlay() {
	
}

void UModuleSystemPanel::execAddModule(FFrame & stack, void * ret) {
	Objects::UObject* module;
	int X, Y, rot;
	stack.stepCompIn(&module);
	stack.stepCompIn(&X);
	stack.stepCompIn(&Y);
	stack.stepCompIn(&rot);
	stack.code += !!stack.code;

	struct Params {
		int y = 0;
		int x = 0;
	};
	Params p;
	module->findFunction(L"getModuleSize")->invoke(module, &p);

	FVector min, max;
	getModuleSpace({(float)X, (float)Y}, rot, {(float)p.x, (float)p.y}, min, max);
	for (int x = min.X; x <= max.X; ++x) for (int y = min.Y; y <= max.Y; ++y) {
		this->grid[x][y] = (SDK::AActor*)module;
	}
}

void UModuleSystemPanel::execRemoveModule(FFrame & stack, void * ret) {
	Objects::UObject* module;
	stack.stepCompIn(&module);
	stack.code += !!stack.code;
	
	for (int x = 0; x < panelHeight; ++x) for (int y = 0; y < panelWidth; ++y) {
		auto& s = this->grid[x][y];
		if (s == (SDK::AActor*)module) s = nullptr;
	}
}

void UModuleSystemPanel::execGetModule(SML::Objects::FFrame & stack, void * ret) {
	int x, y;
	stack.stepCompIn(&x);
	stack.stepCompIn(&y);

	stack.code += !!stack.code;

	*((SDK::AActor**)ret) = getModule(x,y);
}

void UModuleSystemPanel::execGetModules(SML::Objects::FFrame & stack, void * ret) {
	SML::Objects::TArray<SDK::AActor*> holder;
	auto& modules = stack.stepCompInRef<SML::Objects::TArray<SDK::AActor*>>(&holder);
	stack.code += !!stack.code;

	for (int x = 0; x < panelHeight; ++x) {
		for (int y = 0; y < panelWidth; ++y) {
			SDK::AActor* m = getModule(x, y);
			if (m && !(std::find(modules.begin(), modules.end(), m) != modules.end())) {
				modules.add(m);
			}
		}
	}
}

void UModuleSystemPanel::execGetDismantleRefund(SML::Objects::FFrame& stack, void* ret) {
	SML::Objects::TArray<SDK::FInventoryStack> holder;
	auto& refund = stack.stepCompInRef<SML::Objects::TArray<SDK::FInventoryStack>>(&holder);
	
	stack.code += !!stack.code;

	std::set<void*> modules;
	for (int x = 0; x < panelWidth; ++x) for (int y = 0; y < panelHeight; ++y) {
		auto m = (SML::Objects::UObject*) grid[x][y];
		if (m && modules.find(m) == modules.end()) {
			modules.insert(m);
			if (m->clazz->implements((SML::Objects::UClass*)SDK::UFGDismantleInterface::StaticClass())) {
				m->findFunction(L"GetDismantleRefund")->invoke(m, &refund);
			}
		}
	}
}

void UModuleSystemPanel::getModuleSpace(FVector loc, int rot, FVector msize, FVector& min, FVector& max) {
	auto s = msize - 1;
	switch (rot) {
	case 0:
		min = loc;
		max = loc + s;
		break;
	case 1:
		min = {loc.X - s.Y, loc.Y};
		max = {loc.X, loc.Y + s.X};
		break;
	case 2:
		min = loc - s;
		max = loc;
		break;
	case 3:
		min = {loc.X, loc.Y - s.X};
		max = {loc.X + s.Y, loc.Y};
		break;
	}
}


UClass * UModuleSystemPanel::staticClass() {
	return Paks::ClassBuilder<UModuleSystemPanel>::staticClass();
}
