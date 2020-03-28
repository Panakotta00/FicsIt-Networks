#include "FINModuleSystemPanel.h"

#include "FGDismantleInterface.h"

UFINModuleSystemPanel::UFINModuleSystemPanel() {}

UFINModuleSystemPanel::~UFINModuleSystemPanel() {}

void UFINModuleSystemPanel::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);

	for (int i = 0; i < PanelWidth; ++i) delete[] grid[i];
	delete[] grid;
	grid = nullptr;
}

void UFINModuleSystemPanel::PostLoad() {
	Super::PostLoad();

	SetupGrid();
}

void UFINModuleSystemPanel::BeginPlay() {
	Super::BeginPlay();

	SetupGrid();
}

bool UFINModuleSystemPanel::AddModule(AActor* module, int x, int y, int rot) {
	SetupGrid();

	int w, h;
	Cast<IFINModuleSystemModule>(module)->Execute_getModuleSize(module, w, h);

	FVector min, max;
	getModuleSpace({(float)x, (float)y, 0.0f}, rot, {(float)w, (float)h, 0.0f}, min, max);
	for (int x = (int)min.X; x <= max.X; ++x) for (int y = (int)min.Y; y <= max.Y; ++y) {
		grid[x][y] = module;
	}

	OnModuleChanged.Broadcast(module, true);
	return true;
}

bool UFINModuleSystemPanel::RemoveModule(AActor* module) {
	if (!grid) return false;

	bool removed = false;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		auto& s = grid[x][y];
		if (s == module) {
			s = nullptr;
			removed = true;
		}
	}

	if (removed) OnModuleChanged.Broadcast(module, false);

	return removed;
}

AActor* UFINModuleSystemPanel::GetModule(int x, int y) const {
	if (!grid) return nullptr;
	return  (x >= 0 && x < PanelHeight && y >= 0 && y < PanelWidth) ? grid[x][y] : nullptr;;
}

void UFINModuleSystemPanel::GetModules(TArray<AActor*>& modules) {
	for (int x = 0; x < PanelHeight; ++x) {
		for (int y = 0; y < PanelWidth; ++y) {
			auto m = GetModule(x, y);
			if (m && !modules.Contains(m)) {
				modules.Add(m);
			}
		}
	}
}

void UFINModuleSystemPanel::GetDismantleRefund(TArray<FInventoryStack>& refund) {
	TSet<AActor*> modules;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		auto m = GetModule(x, y);
		if (m && modules.Contains(m)) {
			modules.Add(m);
			if (m->Implements<UFGDismantleInterface>()) {
				Cast<IFGDismantleInterface>(m)->Execute_GetDismantleRefund(m, refund);
			}
		}
	}
}

void UFINModuleSystemPanel::SetupGrid() {
	if (grid == nullptr) {
		grid = new AActor**[PanelHeight]();
		for (int i = 0; i < PanelHeight; ++i) {
			grid[i] = new AActor*[PanelWidth];
			memset(grid[i], 0, PanelWidth * sizeof(void*));
		}
	}
}

void UFINModuleSystemPanel::getModuleSpace(FVector loc, int rot, FVector msize, FVector& min, FVector& max) {
	auto s = msize - 1;
	switch (rot) {
	case 0:
		min = loc;
		max = loc + s;
		break;
	case 1:
		min = {loc.X - s.Y, loc.Y, 0.0f};
		max = {loc.X, loc.Y + s.X, 0.0f};
		break;
	case 2:
		min = loc - s;
		max = loc;
		break;
	case 3:
		min = {loc.X, loc.Y - s.X, 0.0f};
		max = {loc.X + s.Y, loc.Y, 0.0f};
		break;
	}
}
