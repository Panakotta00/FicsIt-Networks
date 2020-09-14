#include "FINComputerScreen.h"


#include "UnrealNetwork.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"

void AFINComputerScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerScreen, GPU);
}

AFINComputerScreen::AFINComputerScreen() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINComputerScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFINNetworkTrace());
}

void AFINComputerScreen::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (bWasGPUValid != GPU.IsValid()) {
		OnGPUValidChanged(bWasGPUValid);
		bWasGPUValid = !bWasGPUValid;
	}
}

bool AFINComputerScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerScreen::BindGPU(const FFINNetworkTrace& gpu) {
	if (gpu.GetUnderlyingPtr().IsValid()) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (!(GPU == gpu)) {
		FFINNetworkTrace oldGPU = GPU;
		GPU = FFINNetworkTrace();
		if (oldGPU.GetUnderlyingPtr().IsValid()) Cast<IFINGPUInterface>(oldGPU.GetUnderlyingPtr().Get())->BindScreen(FFINNetworkTrace());
		GPU = gpu;
		if (gpu.GetUnderlyingPtr().IsValid()) Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr().Get())->BindScreen(gpu / this);
		bWasGPUValid = GPU.IsValid();
	}
	NetMulti_OnGPUUpdate();
}

FFINNetworkTrace AFINComputerScreen::GetGPU() const {
	return GPU;
}

void AFINComputerScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINComputerScreen::GetWidget() const {
	return Widget;
}

void AFINComputerScreen::OnGPUValidChanged_Implementation(bool bWasGPUValid) {
	if (bWasGPUValid) {
		if (GPU.GetUnderlyingPtr().IsValid()) Cast<IFINGPUInterface>(GPU.GetUnderlyingPtr().Get())->DropWidget();
	} else {
		Cast<IFINGPUInterface>(GPU.GetUnderlyingPtr().Get())->RequestNewWidget();
	}
}

void AFINComputerScreen::NetMulti_OnGPUUpdate_Implementation() {
	OnGPUUpdate.Broadcast();
	if (GPU) Cast<IFINGPUInterface>(GPU.Get())->RequestNewWidget();
	else SetWidget(nullptr);
}
