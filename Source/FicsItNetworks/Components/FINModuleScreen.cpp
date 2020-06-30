#include "FINModuleScreen.h"

#include "Graphics/FINGraphicsProcessor.h"

AFINModuleScreen::AFINModuleScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFINModuleScreen::BeginPlay() {
	Super::BeginPlay();
	
	if (IsValid(GPU)) Cast<IFINGraphicsProcessor>(GPU)->RequestNewWidget();
}

void AFINModuleScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGraphicsProcessor::StaticClass()))
    if (GPU != gpu) {
    	if (!gpu) SetWidget(nullptr);
    	UObject* oldGPU = GPU;
    	GPU = nullptr;
    	if (oldGPU) Cast<IFINGraphicsProcessor>(oldGPU)->BindScreen(nullptr);
    	GPU = gpu;
    	if (gpu) {
    		Cast<IFINGraphicsProcessor>(gpu)->BindScreen(this);
    		Cast<IFINGraphicsProcessor>(gpu)->RequestNewWidget();
    	}
    }
	OnGPUUpdate.Broadcast();
}

UObject* AFINModuleScreen::GetGPU() const {
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
