#include "FINComputerScreen.h"


#include "UnrealNetwork.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"

void AFINComputerScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerScreen, GPUPtr);
	DOREPLIFETIME(AFINComputerScreen, GPU);
}

AFINComputerScreen::AFINComputerScreen() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINComputerScreen::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) GPUPtr = GPU.Get();
}

void AFINComputerScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFINNetworkTrace());
}

void AFINComputerScreen::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (HasAuthority() && (((bool)GPUPtr) != GPU.IsValid())) {
		if (!GPUPtr) GPUPtr = GPU.Get();
		OnGPUValidChanged(GPU.IsValid(), GPUPtr);
		GPUPtr = GPU.Get();
		ForceNetUpdate();
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
		GPUPtr = GPU.Get();
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

void AFINComputerScreen::RequestNewWidget() {
	if (GPUPtr) Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();
}

void AFINComputerScreen::OnGPUValidChanged_Implementation(bool bValid, UObject* newGPU) {
	if (!bValid) {
		Cast<IFINGPUInterface>(newGPU)->DropWidget();
	} else {
		Cast<IFINGPUInterface>(newGPU)->RequestNewWidget();
	}
}

void AFINComputerScreen::NetMulti_OnGPUUpdate_Implementation() {
	OnGPUUpdate.Broadcast();
	if (GPUPtr) Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();
	else SetWidget(nullptr);
}
