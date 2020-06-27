#include "FINComputerScreen.h"

#include "FicsItNetworks/Graphics/FINGraphicsProcessor.h"

bool AFINComputerScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGraphicsProcessor::StaticClass()))
	if (GPU != gpu) {
		if (!gpu) SetWidget(nullptr);
		UObject* oldGPU = GPU;
		GPU = nullptr;
		if (oldGPU) Cast<IFINGraphicsProcessor>(oldGPU)->BindScreen(nullptr);
		GPU = gpu;
		if (gpu) Cast<IFINGraphicsProcessor>(gpu)->BindScreen(this);
	}
	OnGPUUpdate.Broadcast();
}

UObject* AFINComputerScreen::GetGPU() const {
	return GPU;
}

AFINComputerScreen* UFINScreenWidget::GetScreen() {
	return Screen;
}

void AFINComputerScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	OnWidgetUpdate.Broadcast();
}

void UFINScreenWidget::OnNewWidget() {
	if (Container.IsValid()) {
		if (Screen && Screen->Widget.IsValid()) {
			Container->SetContent(Screen->Widget.ToSharedRef());
		} else {
			Container->SetContent(SNew(SBox));
		}
	}
}

void UFINScreenWidget::OnNewGPU() {
	if (this->Screen->GetGPU()) {
		Cast<IFINGraphicsProcessor>(this->Screen->GetGPU())->RequestNewWidget();
	}
}

void UFINScreenWidget::SetScreen(AFINComputerScreen* Screen) {
	if (this->Screen) {
		this->Screen->OnWidgetUpdate.RemoveDynamic(this, &UFINScreenWidget::OnNewWidget);
		this->Screen->OnGPUUpdate.RemoveDynamic(this, &UFINScreenWidget::OnNewGPU);
	}
	this->Screen = Screen;
	if (this->Screen) {
		this->Screen->OnWidgetUpdate.AddDynamic(this, &UFINScreenWidget::OnNewWidget);
		this->Screen->OnGPUUpdate.AddDynamic(this, &UFINScreenWidget::OnNewGPU);
		if (Container.IsValid() && this->Screen->GetGPU()) {
			Cast<IFINGraphicsProcessor>(this->Screen->GetGPU())->RequestNewWidget();
		}
	}
}

void UFINScreenWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	if (Screen) {
		Screen->Widget.Reset();
		IFINGraphicsProcessor* GPU = Cast<IFINGraphicsProcessor>(Screen->GetGPU());
		if (GPU) GPU->DropWidget();
		OnNewWidget();
	}
	Container.Reset();
}

TSharedRef<SWidget> UFINScreenWidget::RebuildWidget() {
	Container = SNew(SBox);
	OnNewWidget();
	return Container.ToSharedRef();
}
