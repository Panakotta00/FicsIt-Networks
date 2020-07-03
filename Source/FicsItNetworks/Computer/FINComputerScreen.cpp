#include "FINComputerScreen.h"

#include "FicsItNetworks/Graphics/FINGPUInterface.h"

void AFINComputerScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(nullptr);
}

bool AFINComputerScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU != gpu) {
		if (!gpu) SetWidget(nullptr);
		UObject* oldGPU = GPU;
		GPU = nullptr;
		if (oldGPU) Cast<IFINGPUInterface>(oldGPU)->BindScreen(nullptr);
		GPU = gpu;
		if (gpu) Cast<IFINGPUInterface>(gpu)->BindScreen(this);
	}
	OnGPUUpdate.Broadcast();
}

UObject* AFINComputerScreen::GetGPU() const {
	return GPU;
}

void AFINComputerScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINComputerScreen::GetWidget() const {
	return Widget;
}
