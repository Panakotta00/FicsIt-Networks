#include "FINComponentUtility.h"
#include "FGPlayerController.h"
#include "Network/FINNetworkAdapter.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

UFINNetworkConnectionComponent* UFINComponentUtility::GetNetworkConnectorFromHit(FHitResult hit) {
	if (!hit.bBlockingHit) return nullptr;

	UFINNetworkConnectionComponent* connector = nullptr;
	FVector pos;

	if (!IsValid(hit.GetActor())) return nullptr;
	
	TArray<UFINNetworkConnectionComponent*> connectors;
	hit.GetActor()->GetComponents<UFINNetworkConnectionComponent>(connectors);

	for (UFINNetworkConnectionComponent* con : connectors) {
		if (!con) continue;

		FVector npos = con->GetComponentLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = con;
		}
	}

	if (connector) return connector;
	
	TArray<UActorComponent*> adapters = hit.GetActor()->GetComponentsByClass(UFINNetworkAdapterReference::StaticClass());
	
	for (UActorComponent* adapterref : adapters) {
		if (!adapterref || !static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref) continue;

		FVector npos = static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref->Connector;
		}
	}

	return connector;
}

void UFINComponentUtility::ClipboardCopy(FString str) {
	FWindowsPlatformApplicationMisc::ClipboardCopy(*str);
}
