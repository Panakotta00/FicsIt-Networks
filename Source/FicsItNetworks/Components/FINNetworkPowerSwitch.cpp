#include "FINNetworkPowerSwitch.h"

AFINNetworkPowerSwitch::AFINNetworkPowerSwitch() {
	PowerConnection1 = CreateDefaultSubobject<UFGPowerConnectionComponent>("PowerConnection1");
	PowerConnection1->SetupAttachment(RootComponent);
	PowerConnection2 = CreateDefaultSubobject<UFGPowerConnectionComponent>("PowerConnection2");
	PowerConnection2->SetupAttachment(RootComponent);
	PowerInfo1 = CreateDefaultSubobject<UFGPowerInfoComponent>("PowerInfo1");
	PowerInfo2 = CreateDefaultSubobject<UFGPowerInfoComponent>("PowerInfo2");
	PowerConnection1->SetPowerInfo(PowerInfo1);
	PowerConnection2->SetPowerInfo(PowerInfo2);

	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	if (HasAuthority()) PrimaryActorTick.SetTickFunctionEnable(true);
}

void AFINNetworkPowerSwitch::BeginPlay() {
	Super::BeginPlay();

	if (bConnected) PowerConnection1->AddHiddenConnection(PowerConnection2);
}

void AFINNetworkPowerSwitch::Tick(float dt) {
	Super::Tick(dt);

	if (bConnectedHasChanged) OnConnectedChanged();
}

bool AFINNetworkPowerSwitch::ShouldSave_Implementation() const {
	return true;
}

void AFINNetworkPowerSwitch::SetConnected(bool bNewConnected) {
	if (bConnected != bNewConnected) {
		bConnected = bNewConnected;
		bConnectedHasChanged = true;
		if (bConnected) {
			PowerConnection1->AddHiddenConnection(PowerConnection2);
		} else {
			TArray<UFGCircuitConnectionComponent*> connections;
			PowerConnection1->GetHiddenConnections(connections);
			if (connections.Contains(PowerConnection2)) PowerConnection1->RemoveHiddenConnection(PowerConnection2);
		}
	}
}

void AFINNetworkPowerSwitch::OnConnectedChanged_Implementation() {
	bConnectedHasChanged = false;
}