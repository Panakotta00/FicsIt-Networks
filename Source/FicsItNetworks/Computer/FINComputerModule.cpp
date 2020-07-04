#include "FINComputerModule.h"

void AFINComputerModule::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	Ar << Listeners;
}

void AFINComputerModule::EndPlay(EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
	if (ModulePanel) ModulePanel->RemoveModule(this);
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

void AFINComputerModule::AddListener_Implementation(FFINNetworkTrace listener) {
	if (Listeners.Contains(listener)) return;
	Listeners.Add(listener);
}

void AFINComputerModule::RemoveListener_Implementation(FFINNetworkTrace listener) {
	Listeners.Remove(listener);
}

TSet<FFINNetworkTrace> AFINComputerModule::GetListeners_Implementation() {
	return Listeners;
}

UObject* AFINComputerModule::GetSignalSenderOverride_Implementation() {
	return this;
}