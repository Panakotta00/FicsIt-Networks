#include "Components/FINModuleScreen.h"
#include "Graphics/FINGPUInterface.h"
#include "Widgets/Layout/SScaleBox.h"

AFINModuleScreen::AFINModuleScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINModuleScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFIRTrace());
}

void AFINModuleScreen::BindGPU(const FFIRTrace& gpu) {
	if (gpu.IsValidPtr()) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU == gpu) return;
	
	FFIRTrace oldGPU = GPU;
	GPU = FFIRTrace();
	if (oldGPU.IsValidPtr()) Cast<IFINGPUInterface>(oldGPU.GetUnderlyingPtr())->BindScreen(FFIRTrace());

	GPU = gpu;
	if (gpu.IsValidPtr()) Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr())->BindScreen(gpu / this);

	OnGPUUpdate.Broadcast();

#if UE_GAME
	if (!gpu.IsValidPtr()) WidgetComponent->SetSlateWidget(nullptr);
	else Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr())->RequestNewWidget();
#endif
}

FFIRTrace AFINModuleScreen::GetGPU() const {
	return GPU;
}

void AFINModuleScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget == widget) return;
	Widget = widget;
	WidgetComponent->SetSlateWidget(widget);
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINModuleScreen::GetWidget() const {
	return Widget;
}
