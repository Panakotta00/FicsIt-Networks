#include "FINComputerGPU.h"

#include "Graphics/FINScreen.h"

AFINComputerGPU::AFINComputerGPU() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGPU::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (shouldCreate) {
		shouldCreate = false;
		if (!IsValid(Screen)) return;
		if (!Widget.IsValid()) Widget = CreateWidget();
		Cast<IFINScreen>(Screen)->SetWidget(Widget);
	}
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
	shouldCreate = true;
}

void AFINComputerGPU::DropWidget() {
	Widget.Reset();
}

TSharedPtr<SWidget> AFINComputerGPU::CreateWidget() {
	return nullptr;
}

void AFINComputerGPU::netSig_ScreenBound_Implementation(UObject* oldScreen) {}
