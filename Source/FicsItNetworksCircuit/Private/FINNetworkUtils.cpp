#include "FINNetworkUtils.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkComponent.h"

UObject* UFINNetworkUtils::FindNetworkComponentFromObject(UObject* Obj) {
	if (!Obj) return nullptr;
	if (Obj->Implements<UFINNetworkComponent>()) return Obj;
	if (Obj->IsA<AActor>()) {
		TArray<UActorComponent*> Connectors = Cast<AActor>(Obj)->GetComponentsByClass(UFINAdvancedNetworkConnectionComponent::StaticClass());
		for (UActorComponent* Connector : Connectors) {
			if (Connector->Implements<UFINNetworkComponent>()) return Connector;
		}
		TArray<UActorComponent*> Adapters = Cast<AActor>(Obj)->GetComponentsByClass(UFINNetworkAdapterReference::StaticClass());
		if (Adapters.Num() > 0) return Cast<UFINNetworkAdapterReference>(Adapters[0])->Ref->Connector;
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
