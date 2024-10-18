#include "FINNetworkUtils.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkComponent.h"

UObject* UFINNetworkUtils::FindNetworkComponentFromObject(UObject* Obj) {
	if (!Obj) return nullptr;
	if (Obj->Implements<UFINNetworkComponent>()) return Obj;
	if (Obj->IsA<AActor>()) {
		TArray<UFINAdvancedNetworkConnectionComponent*> Connectors;
		Cast<AActor>(Obj)->GetComponents<UFINAdvancedNetworkConnectionComponent>(Connectors);
		for (UFINAdvancedNetworkConnectionComponent* Connector : Connectors) {
			return Connector;
		}
		TArray<UFINNetworkAdapterReference*> Adapters;
		Cast<AActor>(Obj)->GetComponents<UFINNetworkAdapterReference>(Adapters);
		if (Adapters.Num() > 0) return Adapters[0]->Ref->Connector;
	}
	return nullptr;
}

FFIRTrace UFINNetworkUtils::RedirectIfPossible(const FFIRTrace& Trace) {
	UObject* Obj = Trace.GetUnderlyingPtr();
	if (Obj && Obj->Implements<UFINNetworkComponent>()) {
		UObject* RedirectObj = IFINNetworkComponent::Execute_GetInstanceRedirect(Obj);
		if (RedirectObj) return Trace / RedirectObj;
	}
	return Trace;
}


UFINNetworkConnectionComponent* UFINNetworkUtils::GetNetworkConnectorFromHit(FHitResult hit) {
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

	TArray<UFINNetworkAdapterReference*> adapters;
	hit.GetActor()->GetComponents<UFINNetworkAdapterReference>(adapters);

	for (UFINNetworkAdapterReference* adapterref : adapters) {
		if (!adapterref || !adapterref->Ref) continue;

		FVector npos = adapterref->Ref->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = adapterref->Ref->Connector;
		}
	}

	return connector;
}
