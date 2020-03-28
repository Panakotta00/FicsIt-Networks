#include "FINComputerModule.h"

void AFINComputerModule::BeginPlay() {
	Execute_setPanel(this, ModulePanel, (int)ModulePos.X, (int)ModulePos.Y, (int)ModulePos.Z);
	Super::BeginPlay();
}

void AFINComputerModule::EndPlay(EEndPlayReason::Type reason) {
	if (ModulePanel) ModulePanel->RemoveModule(this);
}

bool AFINComputerModule::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerModule::setPanel_Implementation(UFINModuleSystemPanel* panel, int x, int y, int rot) {
	ModulePanel = panel;
	ModulePos = FVector((float)x, (float)y, (float)rot);
	if (IsValid(ModulePanel)) ModulePanel->AddModule(this, x, y, rot);
}

void AFINComputerModule::getModuleSize_Implementation(int& width, int& height) const {
	width = (int) ModuleSize.X;
	height = (int) ModuleSize.Y;
}

FName AFINComputerModule::getName_Implementation() const {
	return ModuleName;
}
