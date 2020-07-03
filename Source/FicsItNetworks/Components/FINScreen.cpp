#include "FINScreen.h"

#include "Graphics/FINGPUInterface.h"

AFINScreen::AFINScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Connector = CreateDefaultSubobject<UFINNetworkConnector>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFINScreen::BeginPlay() {
	Super::BeginPlay();
	
	if (IsValid(GPU)) Cast<IFINGPUInterface>(GPU)->RequestNewWidget();
}

void AFINScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(nullptr);
}

bool AFINScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
    if (GPU != gpu) {
    	if (!gpu) SetWidget(nullptr);
    	UObject* oldGPU = GPU;
    	GPU = nullptr;
    	if (oldGPU) Cast<IFINGPUInterface>(oldGPU)->BindScreen(nullptr);
    	GPU = gpu;
    	if (gpu) {
    		Cast<IFINGPUInterface>(gpu)->BindScreen(this);
    		Cast<IFINGPUInterface>(gpu)->RequestNewWidget();
    	}
    }
	OnGPUUpdate.Broadcast();
}

UObject* AFINScreen::GetGPU() const {
	return GPU;
}

void AFINScreen::SetWidget(TSharedPtr<SWidget> widget) {
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

TSharedPtr<SWidget> AFINScreen::GetWidget() const {
	return Widget;
}
