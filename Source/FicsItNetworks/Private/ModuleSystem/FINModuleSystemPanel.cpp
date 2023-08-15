#include "ModuleSystem/FINModuleSystemPanel.h"
#include "FGDismantleInterface.h"
#include "ModuleSystem/FINModuleSystemModule.h"

void UFINModuleSystemPanel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFINModuleSystemPanel, AllowedModules);
	DOREPLIFETIME(UFINModuleSystemPanel, Grid);
}

void UFINModuleSystemPanel::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	if (Ar.IsSaveGame()) {
		SetupGrid();

		int height = PanelHeight, width = PanelWidth;
		Ar << PanelHeight;
		Ar << PanelWidth;

		for (int x = 0; x < PanelHeight; ++x) {
			for (int y = 0; y < PanelWidth; ++y) {
				if (x < height && y < width) {
					UObject* ptr = GetGridSlot(x, y);
					Ar << ptr;
					GetGridSlot(x, y) = ptr;
				} else {
					UObject* ptr = nullptr;
					Ar << ptr;
				}
			}
		}
	}
}

void UFINModuleSystemPanel::InitializeComponent() {
	Super::InitializeComponent();
	SetIsReplicated(true);
}

void UFINModuleSystemPanel::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
}

bool UFINModuleSystemPanel::ShouldSave_Implementation() const {
	return true;
}

void UFINModuleSystemPanel::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	
}

bool UFINModuleSystemPanel::AddModule(AActor* module, int x, int y, int rot) {
	SetupGrid();

	int w, h;
	IFINModuleSystemModule::Execute_getModuleSize(module, w, h);

	FVector min, max;
	GetModuleSpace({static_cast<float>(x), static_cast<float>(y), 0.0f}, rot, {static_cast<float>(w), static_cast<float>(h), 0.0f}, min, max);
	for (int MX = static_cast<int>(min.X); MX <= max.X; ++MX) for (int MY = static_cast<int>(min.Y); MY <= max.Y; ++MY) {
		GetGridSlot(MX, MY) = module;
	}

	GetOwner()->ForceNetUpdate();
	OnModuleChanged.Broadcast(module, true);
	
	return true;
}

bool UFINModuleSystemPanel::RemoveModule(AActor* Module) {
	bool removed = false;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		auto& s = GetGridSlot(x, y);
		if (s == Module) {
			s = nullptr;
			removed = true;
		}
	}

	if (removed) {
		GetOwner()->ForceNetUpdate();
		OnModuleChanged.Broadcast(Module, false);
	}
	
	return removed;
}

AActor* UFINModuleSystemPanel::GetModule(int x, int y) const {
	if (Grid.Num() < 1) return nullptr;
	return  (x >= 0 && x < PanelHeight && y >= 0 && y < PanelWidth) ? Cast<AActor>(GetGridSlot(x, y)) : nullptr;;
}

void UFINModuleSystemPanel::GetModules(TArray<AActor*>& modules) const {
	if (Grid.Num() < 1) return;
	for (int x = 0; x < PanelHeight; ++x) {
		for (int y = 0; y < PanelWidth; ++y) {
			auto m = GetModule(x, y);
			if (m && !modules.Contains(m)) {
				modules.Add(m);
			}
		}
	}
}

void UFINModuleSystemPanel::GetDismantleRefund(TArray<FInventoryStack>& refund, bool noCost) const {
	if (Grid.Num() < 1) return;
	TSet<AActor*> modules;
	for (int x = 0; x < PanelHeight; ++x) for (int y = 0; y < PanelWidth; ++y) {
		AActor* m = GetModule(x, y);
		if (m && !modules.Contains(m)) {
			modules.Add(m);
			if (m->Implements<UFGDismantleInterface>()) {
				IFGDismantleInterface::Execute_GetDismantleRefund(m, refund, noCost);
			}
		}
	}
}

void UFINModuleSystemPanel::SetupGrid() {
	if (Grid.Num() < 1) {
		for (int i = 0; i < PanelHeight * PanelWidth; ++i) {
			Grid.Add(nullptr);
		}
	}
}

UObject* UFINModuleSystemPanel::GetGridSlot(int x, int y) const {
	return Grid[x*PanelWidth + y];
}

UObject*& UFINModuleSystemPanel::GetGridSlot(int x, int y) {
	return Grid[x*PanelWidth + y];
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
