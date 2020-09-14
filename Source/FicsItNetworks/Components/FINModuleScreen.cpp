#include "FINModuleScreen.h"


#include "UnrealNetwork.h"
#include "Graphics/FINGPUInterface.h"

AFINModuleScreen::AFINModuleScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINModuleScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINModuleScreen, GPU);
}

void AFINModuleScreen::BeginPlay() {
	Super::BeginPlay();
	
	if (GPU.IsValid()) Cast<IFINGPUInterface>(GPU.Get())->RequestNewWidget();
}

void AFINModuleScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFINNetworkTrace());
}

void AFINModuleScreen::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (bWasGPUValid != GPU.IsValid()) {
		OnGPUValidationChanged(bWasGPUValid);
		bWasGPUValid = !bWasGPUValid;
	}
}

void AFINModuleScreen::BindGPU(const FFINNetworkTrace& gpu) {
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

void AFINModuleScreen::OnGPUValidationChanged_Implementation(bool bWasValid) {
	if (bWasValid) {
		if (GPU.GetUnderlyingPtr().IsValid()) Cast<IFINGPUInterface>(GPU.GetUnderlyingPtr().Get())->DropWidget();
	} else {
		Cast<IFINGPUInterface>(GPU.GetUnderlyingPtr().Get())->RequestNewWidget();
	}
}

void AFINModuleScreen::NetMulti_OnGPUUpdate_Implementation() {
	if (GPU.IsValid()) {
		Cast<IFINGPUInterface>(GPU.Get())->RequestNewWidget();
	} else {
		SetWidget(nullptr);
	}
	OnGPUUpdate.Broadcast();
}
