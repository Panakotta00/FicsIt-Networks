#include "FINModuleSystemPanel.h"

#include "FGDismantleInterface.h"

UFINModuleSystemPanel::UFINModuleSystemPanel() : grid(nullptr) {}

UFINModuleSystemPanel::~UFINModuleSystemPanel() {}

void UFINModuleSystemPanel::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	if (Ar.IsSaveGame()) {
		SetupGrid();

		int height = PanelHeight, width = PanelWidth;
		Ar << height;
		Ar << width;
	
		for (int x = 0; x < height; ++x) {
			for (int y = 0; y < width; ++y) {
				if (x < PanelHeight && y < PanelHeight) {
					Ar << grid[x][y];
				} else {
					UObject* ptr = nullptr;
					Ar << ptr;
				}
			}
		}
	}
}

void UFINModuleSystemPanel::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);

	if (grid == nullptr) return;
	for (int i = 0; i < PanelWidth; ++i) delete[] grid[i];
	delete[] grid;
	grid = nullptr;
}

bool UFINModuleSystemPanel::ShouldSave_Implementation() const {
	return true;
}

void UFINModuleSystemPanel::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	
}

bool UFINModuleSystemPanel::AddModule(AActor* module, int x, int y, int rot) {
	SetupGrid();

	int w, h;
	Cast<IFINModuleSystemModule>(module)->Execute_getModuleSize(module, w, h);

	FVector min, max;
	GetModuleSpace({static_cast<float>(x), static_cast<float>(y), 0.0f}, rot, {static_cast<float>(w), static_cast<float>(h), 0.0f}, min, max);
	for (int MX = static_cast<int>(min.X); MX <= max.X; ++MX) for (int MY = static_cast<int>(min.Y); MY <= max.Y; ++MY) {
		grid[MX][MY] = module;
	}

	OnModuleChanged.Broadcast(module, true);
	return true;
}

bool UFINModuleSystemPanel::RemoveModule(AActor* Module) {
	if (!grid) return false;

	bool removed = false;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		auto& s = grid[x][y];
		if (s == Module) {
			s = nullptr;
			removed = true;
		}
	}

	if (removed) OnModuleChanged.Broadcast(Module, false);

	return removed;
}

AActor* UFINModuleSystemPanel::GetModule(int x, int y) const {
	if (!grid) return nullptr;
	return  (x >= 0 && x < PanelHeight && y >= 0 && y < PanelWidth) ? Cast<AActor>(grid[x][y].Get()) : nullptr;;
}

void UFINModuleSystemPanel::GetModules(TArray<AActor*>& modules) const {
	for (int x = 0; x < PanelHeight; ++x) {
		for (int y = 0; y < PanelWidth; ++y) {
			auto m = GetModule(x, y);
			if (m && !modules.Contains(m)) {
				modules.Add(m);
			}
		}
	}
}

void UFINModuleSystemPanel::GetDismantleRefund(TArray<FInventoryStack>& refund) const {
	TSet<AActor*> modules;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		AActor* m = GetModule(x, y);
		if (m && !modules.Contains(m)) {
			modules.Add(m);
			if (m->Implements<UFGDismantleInterface>()) {
				IFGDismantleInterface::Execute_GetDismantleRefund(m, refund);
			}
		}
	}
}

void UFINModuleSystemPanel::SetupGrid() {
	if (grid == nullptr) {
		grid = new FWeakObjectPtr*[PanelHeight]();
		for (int i = 0; i < PanelHeight; ++i) {
			grid[i] = new FWeakObjectPtr[PanelWidth];
			memset(grid[i], 0, PanelWidth * sizeof(void*));
		}
	}
}

void UFINModuleSystemPanel::GetModuleSpace(const FVector& Loc, const int Rot, const FVector& MSize, FVector& OutMin, FVector& OutMax) {
	const FVector s = MSize - 1;
	switch (Rot) {
	case 0:
		OutMin = Loc;
		OutMax = Loc + s;
		break;
	case 1:
		OutMin = {Loc.X - s.Y, Loc.Y, 0.0f};
		OutMax = {Loc.X, Loc.Y + s.X, 0.0f};
		break;
	case 2:
		OutMin = Loc - s;
		OutMax = Loc;
		break;
	case 3:
		OutMin = {Loc.X, Loc.Y - s.X, 0.0f};
		OutMax = {Loc.X + s.Y, Loc.Y, 0.0f};
		break;
	default:
		break;
	}
}
