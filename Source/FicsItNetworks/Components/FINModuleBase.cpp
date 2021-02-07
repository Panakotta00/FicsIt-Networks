#include "FINModuleBase.h"


#include "UnrealNetwork.h"
#include "ModuleSystem/FINModuleSystemHolo.h"

void AFINModuleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINModuleBase, ModuleName);
}

void AFINModuleBase::EndPlay(EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
	if (HasAuthority() && ModulePanel) ModulePanel->RemoveModule(this);
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

UObject* AFINModuleBase::GetSignalSenderOverride_Implementation() {
	UObject* obj = Cast<UObject>(this);
	return obj;
}
