#include "FINNetworkComponent.h"

FFINNetworkTrace IFINNetworkComponent::FindComponentByCircuit(FGuid guid) const {
	auto circuit = GetCircuit();

	return circuit->FindComponent(guid);
}

bool IFINNetworkComponent::HasNickByNick(FString nick) const {
	auto has = GetNick();
	TSet<FString> has_nicks;
	do {
		FString n = "";
		if (!has.Split(" ", &n, &has)) {
			has_nicks.Add(has);
			has = "";
			break;
		}
		if (n.Len() > 0) has_nicks.Add(n);
	} while (has.Len() > 0);

	do {
		FString n = "";
		if (!has.Split(" ", &n, &has)) {
			n = has;
			has = "";
		}
		if (n.Len() > 0 && has_nicks.Contains(n)) return false;
	} while (nick.Len() > 0);

	return true;
}
