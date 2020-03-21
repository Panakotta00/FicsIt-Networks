#include "FINNetworkPowerSwitch.h"

AFINNetworkPowerSwitch::AFINNetworkPowerSwitch() {
	PowerConnection1 = CreateDefaultSubobject<UFGPowerConnectionComponent>("PowerConnection1");
	PowerConnection2 = CreateDefaultSubobject<UFGPowerConnectionComponent>("PowerConnection2");
	PowerInfo1 = CreateDefaultSubobject<UFGPowerInfoComponent>("PowerInfo1");
	PowerInfo2 = CreateDefaultSubobject<UFGPowerInfoComponent>("PowerInfo2");
	PowerConnection1->SetPowerInfo(PowerInfo1);
	PowerConnection2->SetPowerInfo(PowerInfo2);

	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->AddMerged(this);

	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AFINNetworkPowerSwitch::BeginPlay() {
	OnConnectedChanged();
}

void AFINNetworkPowerSwitch::Tick(float dt) {
	if (bHasChanged) OnConnectedChanged();
}

bool AFINNetworkPowerSwitch::ShouldSave_Implementation() const {
	return true;
}

void AFINNetworkPowerSwitch::SetConnected(bool bNewConnected) {
	if (bConnected != bNewConnected) {
		bConnected = bNewConnected;
		bHasChanged = true;
	}
}

void AFINNetworkPowerSwitch::OnConnectedChanged_Implementation() {
	bHasChanged = false;
	if (bConnected) {
		PowerConnection1->AddHiddenConnection(PowerConnection2);
	} else {
		TArray<UFGCircuitConnectionComponent*> connections;
		PowerConnection1->GetHiddenConnections(connections);
		if (connections.Contains(PowerConnection2)) PowerConnection1->RemoveHiddenConnection(PowerConnection2);
	}
}