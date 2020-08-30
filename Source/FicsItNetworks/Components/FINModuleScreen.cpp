#include "FINModuleScreen.h"


#include "UnrealNetwork.h"
#include "Graphics/FINGPUInterface.h"

AFINModuleScreen::AFINModuleScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFINModuleScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	DOREPLIFETIME(AFINModuleScreen, GPU);
}

void AFINModuleScreen::BeginPlay() {
	Super::BeginPlay();
	
	if (IsValid(GPU)) Cast<IFINGPUInterface>(GPU)->RequestNewWidget();
}

void AFINModuleScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(nullptr);
}

void AFINModuleScreen::BindGPU(UObject* gpu) {
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
