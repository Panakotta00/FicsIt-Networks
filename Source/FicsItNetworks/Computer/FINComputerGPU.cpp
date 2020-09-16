#include "FINComputerGPU.h"


#include "UnrealNetwork.h"
#include "WidgetInteractionComponent.h"
#include "Graphics/FINScreenInterface.h"
#include "Private/KismetTraceUtils.h"

AFINComputerGPU::AFINComputerGPU() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGPU::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerGPU, ScreenPtr);
	DOREPLIFETIME(AFINComputerGPU, Screen);
}

void AFINComputerGPU::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) ScreenPtr = Screen.Get();
}

void AFINComputerGPU::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	DrawDebugLine(this->GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0,0,1000000), FColor::Red, true);
	
	if (HasAuthority() && (((bool)ScreenPtr) != Screen.IsValid())) {
		if (!ScreenPtr) ScreenPtr = Screen.Get();
		OnValidationChanged(Screen.IsValid(), ScreenPtr);
		ScreenPtr = Screen.Get();
		ForceNetUpdate();
	}
	if (bShouldCreate) {
		bShouldCreate = false;
		if (!ScreenPtr) return;
		if (!Widget.IsValid()) Widget = CreateWidget();
		Cast<IFINScreenInterface>(ScreenPtr)->SetWidget(Widget);
	}
	if (bScreenChanged) {
		bScreenChanged = false;
		ForceNetUpdate();
	}
}

void AFINComputerGPU::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindScreen(FFINNetworkTrace());
}

void AFINComputerGPU::BindScreen(const FFINNetworkTrace& screen) {
	if (screen.GetUnderlyingPtr().IsValid()) check(screen->GetClass()->ImplementsInterface(UFINScreenInterface::StaticClass()))
	if (Screen == screen) return;
	FFINNetworkTrace oldScreen = Screen;
	Screen = FFINNetworkTrace();
	if (oldScreen.IsValid()) Cast<IFINScreenInterface>(oldScreen.Get())->BindGPU(FFINNetworkTrace());
	if (!screen.IsValid()) DropWidget();
	Screen = screen;
	if (screen.IsValid()) Cast<IFINScreenInterface>(screen.Get())->BindGPU(screen / this);
	ScreenPtr = screen.Get();
	netSig_ScreenBound(oldScreen);
	bScreenChanged = true;
}

FFINNetworkTrace AFINComputerGPU::GetScreen() const {
	return Screen;
}

void AFINComputerGPU::RequestNewWidget() {
	bShouldCreate = true;
}

void AFINComputerGPU::DropWidget() {
	Widget.Reset();
	if (ScreenPtr) Cast<IFINScreenInterface>(ScreenPtr)->SetWidget(nullptr);
}

void AFINComputerGPU::OnValidationChanged_Implementation(bool bValid, UObject* newScreen) {
	if (!bValid) DropWidget();
	else RequestNewWidget();
}

TSharedPtr<SWidget> AFINComputerGPU::CreateWidget() {
	return nullptr;
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
	if (Screen && Cast<IFINScreenInterface>(Screen)->GetGPU()) {
		Cast<IFINScreenInterface>(Screen)->RequestNewWidget();
	} else if (Container.IsValid()) {
		Container->SetContent(SNew(SBox));
	}
}

void UFINScreenWidget::SetScreen(UObject* NewScreen) {
	Screen = NewScreen;
	if (Screen) {
		if (Container.IsValid()) {
			Cast<IFINScreenInterface>(Screen)->RequestNewWidget();
		}
	}
}

UObject* UFINScreenWidget::GetScreen() {
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

	/*if (Screen) {
		IFINGPUInterface* GPU = Cast<IFINGPUInterface>(Cast<IFINScreenInterface>(Screen)->GetGPU());
		if (GPU) GPU->DropWidget();
		OnNewWidget();
	}
	Container.Reset();*/
}

TSharedRef<SWidget> UFINScreenWidget::RebuildWidget() {
	Container = SNew(SBox);
	OnNewWidget();
	return Container.ToSharedRef();
}
