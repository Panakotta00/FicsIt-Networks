#include "Computer/FINComputerGPU.h"

#include "Buildables/FGBuildableWidgetSign.h"
#include "Graphics/FINScreenInterface.h"

AFINComputerGPU::AFINComputerGPU() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGPU::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerGPU, Screen);
}

void AFINComputerGPU::BeginPlay() {
	Super::BeginPlay();

	UpdateScreen();
}

void AFINComputerGPU::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	// TODO: Continuesly check for Trace Validity and udate widget accordingly
}

void AFINComputerGPU::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindScreen(FFINNetworkTrace());
}

void AFINComputerGPU::BindScreen(const FFINNetworkTrace& screen) {
	if (screen.IsValidPtr()) check(screen->GetClass()->ImplementsInterface(UFINScreenInterface::StaticClass()))
	if (Screen == screen) return;

	Screen = screen;
	UpdateScreen();
}

FFINNetworkTrace AFINComputerGPU::GetScreen() const {
	return Screen;
}

void AFINComputerGPU::RequestNewWidget() {
	if (!GetScreenInterface()) return;
	GetScreenInterface()->SetWidget(CreateWidget());
}

TScriptInterface<IFINScreenInterface> AFINComputerGPU::GetScreenInterface() {
	return Screen.GetUnderlyingPtr();
}

TSharedPtr<SWidget> AFINComputerGPU::CreateWidget() {
	return nullptr;
}

int AFINComputerGPU::MouseToInt(const FPointerEvent& MouseEvent) {
	int mouseEvent = 0;
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))	mouseEvent |= 0b0000000001;
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))	mouseEvent |= 0b0000000010;
	if (MouseEvent.IsControlDown())								mouseEvent |= 0b0000000100;
	if (MouseEvent.IsShiftDown())								mouseEvent |= 0b0000001000;
	if (MouseEvent.IsAltDown())									mouseEvent |= 0b0000010000;
	if (MouseEvent.IsCommandDown())								mouseEvent |= 0b0000100000;
	return mouseEvent;
}

int AFINComputerGPU::InputToInt(const FInputEvent& KeyEvent) {
	int mouseEvent = 0;
	if (KeyEvent.IsControlDown())								mouseEvent |= 0b0000000100;
	if (KeyEvent.IsShiftDown())									mouseEvent |= 0b0000001000;
	if (KeyEvent.IsAltDown())									mouseEvent |= 0b0000010000;
	if (KeyEvent.IsCommandDown())								mouseEvent |= 0b0000100000;
	return mouseEvent;
}

void AFINComputerGPU::OnRep_Screen() {
	UpdateScreen();
}

void AFINComputerGPU::UpdateScreen() {
	FFINNetworkTrace newScreen = Screen;
	Screen = FFINNetworkTrace();
	if (OldScreenCache.IsValidPtr()) Cast<IFINScreenInterface>(OldScreenCache.Get())->BindGPU(FFINNetworkTrace());
	FFINNetworkTrace oldScreen = OldScreenCache;
	OldScreenCache = Screen = newScreen;
	
	if (Screen.IsValidPtr()) Cast<IFINScreenInterface>(Screen.Get())->BindGPU(Screen / this);

	if (HasAuthority()) netSig_ScreenBound(oldScreen);
}

void AFINComputerGPU::netFunc_bindScreen(FFINNetworkTrace NewScreen) {
	if (AFGBuildableWidgetSign* WidgetSign = Cast<AFGBuildableWidgetSign>(*NewScreen)) {
		// TODO: Vanilla Sign Support
	}
	if (Cast<IFINScreenInterface>(NewScreen.GetUnderlyingPtr())) {
		BindScreen(NewScreen);
		return;
	}
	BindScreen(FFINNetworkTrace());
}

FVector2D AFINComputerGPU::netFunc_getScreenSize() {
	TScriptInterface<IFINScreenInterface> screen = GetScreenInterface();
	if (screen) {
		return screen->GetWidget()->GetCachedGeometry().GetLocalSize();
	} else {
		return FVector2D::Zero();
	}
}

void AFINComputerGPU::netSig_ScreenBound_Implementation(const FFINNetworkTrace& oldScreen) {}

void UFINScreenWidget::OnNewWidget() {
	if (Container.IsValid()) {
		if (Screen && Cast<IFINScreenInterface>(Screen)->GetWidget().IsValid()) {
			Container->SetContent(Cast<IFINScreenInterface>(Screen)->GetWidget().ToSharedRef());
		} else {
			Container->SetContent(SNew(SBox));
		}
	}
}

void UFINScreenWidget::OnNewGPU() {
	if (Screen) {
		TScriptInterface<IFINGPUInterface> GPU = GetScreen()->GetGPU().GetUnderlyingPtr();
		if (GPU) {
			GPU->RequestNewWidget();
			return;
		}
	}
	if (Container.IsValid()) Container->SetContent(SNew(SBox));
}

void UFINScreenWidget::SetScreen(UObject* NewScreen) {
	Screen = NewScreen;
	OnNewGPU();
}

TScriptInterface<IFINScreenInterface> UFINScreenWidget::GetScreen() {
	return Screen;
}

void UFINScreenWidget::Focus() {
	if (this->Screen) {
		TSharedPtr<SWidget> widget = Cast<IFINScreenInterface>(this->Screen)->GetWidget();
		if (widget.IsValid()) {
			FSlateApplication::Get().SetKeyboardFocus(widget);
		}
	}
}

void UFINScreenWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	Container.Reset();

	/*if (Screen) {
		IFINGPUInterface* GPU = Cast<IFINGPUInterface>(Cast<IFINScreenInterface>(Screen)->GetGPU());
		if (GPU) GPU->DropWidget();
		OnNewWidget();
	}
	Container.Reset();*/
}

TSharedRef<SWidget> UFINScreenWidget::RebuildWidget() {
	Container = SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill);
	OnNewWidget();
	return Container.ToSharedRef();
}

void UFINGPUWidgetSign::BindGPU(const FFINNetworkTrace& gpu) {}
FFINNetworkTrace UFINGPUWidgetSign::GetGPU() const {
	return GPU;
}

void UFINGPUWidgetSign::SetWidget(TSharedPtr<SWidget> widget) {}
TSharedPtr<SWidget> UFINGPUWidgetSign::GetWidget() const {
	return nullptr;
}