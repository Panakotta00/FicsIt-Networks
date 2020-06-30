#include "FINModuleBase.h"

#include "ModuleSystem/FINModuleSystemHolo.h"

void AFINModuleBase::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	Ar << Listeners;
}

bool AFINModuleBase::ShouldSave_Implementation() const {
	return true;
}

void AFINModuleBase::setPanel_Implementation(UFINModuleSystemPanel* Panel, const int X, const int Y, const int Rot) {
	ModulePanel = Panel;
	ModulePos = FVector(static_cast<float>(X), static_cast<float>(Y), static_cast<float>(Rot));
	if (IsValid(ModulePanel)) ModulePanel->AddModule(this, X, Y, Rot);
}

void AFINModuleBase::getModuleSize_Implementation(int& Width, int& Height) const {
	Width = static_cast<int>(ModuleSize.X);
	Height = static_cast<int>(ModuleSize.Y);
}

FName AFINModuleBase::getName_Implementation() const {
	return ModuleName;
}

void AFINModuleBase::AddListener_Implementation(FFINNetworkTrace listener) {
	if (Listeners.Contains(listener)) return;
	Listeners.Add(listener);
}

void AFINModuleBase::RemoveListener_Implementation(FFINNetworkTrace listener) {
	Listeners.Remove(listener);
}

TSet<FFINNetworkTrace> AFINModuleBase::GetListeners_Implementation() {
	return Listeners;
}

UObject* AFINModuleBase::GetSignalSenderOverride_Implementation() {
	return this;
}