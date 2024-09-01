#include "Computer/FINComputerScreen.h"
#include "Graphics/FINGPUInterface.h"

AFINComputerScreen::AFINComputerScreen() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINComputerScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFIRTrace());
}

void AFINComputerScreen::BindGPU(const FFIRTrace& gpu) {
	if (gpu.IsValidPtr()) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU == gpu) return;
	
	FFIRTrace oldGPU = GPU;
	GPU = FFIRTrace();
	if (oldGPU.IsValidPtr()) Cast<IFINGPUInterface>(oldGPU.GetUnderlyingPtr())->BindScreen(FFIRTrace());

	GPU = gpu;
	if (gpu.IsValidPtr()) Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr())->BindScreen(gpu / this);

	OnGPUUpdate.Broadcast();
}

FFIRTrace AFINComputerScreen::GetGPU() const {
	return GPU;
}

void AFINComputerScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget == widget) return;

	Widget = widget;
	
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINComputerScreen::GetWidget() const {
	return Widget;
}
