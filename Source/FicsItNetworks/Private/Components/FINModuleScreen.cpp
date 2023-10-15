#include "Components/FINModuleScreen.h"
#include "Graphics/FINGPUInterface.h"

AFINModuleScreen::AFINModuleScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINModuleScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFINNetworkTrace());
}

void AFINModuleScreen::BindGPU(const FFINNetworkTrace& gpu) {
	if (gpu.IsValidPtr()) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU == gpu) return;
	
	FFINNetworkTrace oldGPU = GPU;
	GPU = FFINNetworkTrace();
	if (oldGPU.IsValidPtr()) Cast<IFINGPUInterface>(oldGPU.GetUnderlyingPtr())->BindScreen(FFINNetworkTrace());

	GPU = gpu;
	if (gpu.IsValidPtr()) Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr())->BindScreen(gpu / this);

	OnGPUUpdate.Broadcast();
}

FFINNetworkTrace AFINModuleScreen::GetGPU() const {
	return GPU;
}

void AFINModuleScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	WidgetComponent->SetSlateWidget(
		Widget.IsValid() ?
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.Content()[
				Widget.ToSharedRef()
			]
		:
			TSharedPtr<SScaleBox>(nullptr));
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINModuleScreen::GetWidget() const {
	return Widget;
}
