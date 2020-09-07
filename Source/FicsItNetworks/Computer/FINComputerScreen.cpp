#include "FINComputerScreen.h"


#include "UnrealNetwork.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"

void AFINComputerScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerScreen, GPU);
}

void AFINComputerScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(nullptr);
}

bool AFINComputerScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU != gpu) {
		UObject* oldGPU = GPU;
		GPU = nullptr;
		if (oldGPU) Cast<IFINGPUInterface>(oldGPU)->BindScreen(nullptr);
		GPU = gpu;
		if (gpu) Cast<IFINGPUInterface>(gpu)->BindScreen(this);
	}
	NetMulti_OnGPUUpdate();
}

UObject* AFINComputerScreen::GetGPU() const {
	return GPU;
}

#pragma optimize("", off)
void AFINComputerScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	OnWidgetUpdate.Broadcast();
}
#pragma optimize("", on)

TSharedPtr<SWidget> AFINComputerScreen::GetWidget() const {
	return Widget;
}

void AFINComputerScreen::NetMulti_OnGPUUpdate_Implementation() {
	OnGPUUpdate.Broadcast();
	if (GPU) Cast<IFINGPUInterface>(GPU)->RequestNewWidget();
	else SetWidget(nullptr);
}
