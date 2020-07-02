#include "FINComputerGPU.h"


#include "WidgetInteractionComponent.h"
#include "Graphics/FINScreen.h"
#include "Private/KismetTraceUtils.h"

AFINComputerGPU::AFINComputerGPU() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGPU::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	DrawDebugLine(this->GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0,0,1000000), FColor::Red, true);
	
	if (bShouldCreate) {
		bShouldCreate = false;
		if (!IsValid(Screen)) return;
		if (!Widget.IsValid()) Widget = CreateWidget();
		Cast<IFINScreen>(Screen)->SetWidget(Widget);
	}
}

void AFINComputerGPU::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (endPlayReason == EEndPlayReason::Destroyed) BindScreen(nullptr);
}

bool AFINComputerGPU::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerGPU::BindScreen(UObject* screen) {
	if (screen) check(screen->GetClass()->ImplementsInterface(UFINScreen::StaticClass()))
	if (Screen == screen) return;
	UObject* oldScreen = Screen;
	Screen = nullptr;
	if (oldScreen) Cast<IFINScreen>(oldScreen)->BindGPU(nullptr);
	Screen = screen;
	if (screen) Cast<IFINScreen>(screen)->BindGPU(this);
	if (!screen) DropWidget();
	netSig_ScreenBound(oldScreen);
}

UObject* AFINComputerGPU::GetScreen() const {
	return Screen;
}

void AFINComputerGPU::RequestNewWidget() {
	bShouldCreate = true;
}

void AFINComputerGPU::DropWidget() {
	Widget.Reset();
	if (Screen) Cast<IFINScreen>(Screen)->SetWidget(nullptr);
}

TSharedPtr<SWidget> AFINComputerGPU::CreateWidget() {
	return nullptr;
}

void AFINComputerGPU::netSig_ScreenBound_Implementation(UObject* oldScreen) {}


void UFINScreenWidget::OnNewWidget() {
	if (Container.IsValid()) {
		if (Screen && Cast<IFINScreen>(Screen)->GetWidget().IsValid()) {
			Container->SetContent(Cast<IFINScreen>(Screen)->GetWidget().ToSharedRef());
		} else {
			Container->SetContent(SNew(SBox));
		}
	}
}

void UFINScreenWidget::OnNewGPU() {
	if (this->Screen && Cast<IFINScreen>(this->Screen)->GetGPU()) {
		Cast<IFINGraphicsProcessor>(Cast<IFINScreen>(this->Screen)->GetGPU())->RequestNewWidget();
	} else if (Container.IsValid()) {
		Container->SetContent(SNew(SBox));
	}
}

void UFINScreenWidget::SetScreen(UObject* Screen) {
	this->Screen = Screen;
	if (this->Screen) {
		if (Container.IsValid() && Cast<IFINScreen>(this->Screen)->GetGPU()) {
			Cast<IFINGraphicsProcessor>(Cast<IFINScreen>(this->Screen)->GetGPU())->RequestNewWidget();
		}
	}
}

UObject* UFINScreenWidget::GetScreen() {
	return Screen;
}

void UFINScreenWidget::Focus() {
	if (this->Screen) {
		TSharedPtr<SWidget> widget = Cast<IFINScreen>(this->Screen)->GetWidget();
		if (widget.IsValid()) {
			FSlateApplication::Get().SetKeyboardFocus(widget);
		}
	}
}

void UFINScreenWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	if (Screen) {
		IFINGraphicsProcessor* GPU = Cast<IFINGraphicsProcessor>(Cast<IFINScreen>(Screen)->GetGPU());
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
