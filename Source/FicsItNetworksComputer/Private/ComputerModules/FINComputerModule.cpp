#include "ComputerModules/FINComputerModule.h"

#include "ModuleSystem/FINModuleSystemPanel.h"

void AFINComputerModule::EndPlay(EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
	if (reason == EEndPlayReason::Destroyed && ModulePanel) ModulePanel->RemoveModule(this);
}

bool AFINComputerModule::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerModule::setPanel_Implementation(UFINModuleSystemPanel* Panel, const int X, const int Y, const int Rot) {
	ModulePanel = Panel;
	ModulePos = FVector(static_cast<float>(X), static_cast<float>(Y), static_cast<float>(Rot));
	if (IsValid(ModulePanel)) ModulePanel->AddModule(this, X, Y, Rot);
}

void AFINComputerModule::getModuleSize_Implementation(int& Width, int& Height) const {
	Width = static_cast<int>(ModuleSize.X);
	Height = static_cast<int>(ModuleSize.Y);
}

FName AFINComputerModule::getName_Implementation() const {
	return ModuleName;
}

UObject* AFINComputerModule::GetSignalSenderOverride_Implementation() {
	return this;
}