#include "FINNetworkUtils.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkComponent.h"
#include "EngineUtils.h"

TArray<AFINNetworkAdapter*> UFINNetworkUtils::GetAttachedNetworkAdapters(AActor* Actor) {
	TArray<AFINNetworkAdapter*> Adapters;
	if (!IsValid(Actor)) return Adapters;

	TArray<UFINNetworkAdapterReference*> AdapterRefs;
	Actor->GetComponents<UFINNetworkAdapterReference>(AdapterRefs);
	for (UFINNetworkAdapterReference* AdapterRef : AdapterRefs) {
		if (AdapterRef && IsValid(AdapterRef->Ref)) Adapters.AddUnique(AdapterRef->Ref);
	}

	if (Adapters.Num() > 0 || Actor->GetNetMode() != NM_Client) return Adapters;
	for (TActorIterator<AFINNetworkAdapter> It(Actor->GetWorld()); It; ++It) {
		if (It->Parent == Actor) Adapters.Add(*It);
	}
	return Adapters;
}

UObject* UFINNetworkUtils::FindNetworkComponentFromObject(UObject* Obj) {
	if (!Obj) return nullptr;
	if (Obj->Implements<UFINNetworkComponent>()) return Obj;
	if (Obj->IsA<AActor>()) {
		TArray<UFINAdvancedNetworkConnectionComponent*> Connectors;
		Cast<AActor>(Obj)->GetComponents<UFINAdvancedNetworkConnectionComponent>(Connectors);
		for (UFINAdvancedNetworkConnectionComponent* Connector : Connectors) {
			return Connector;
		}
		TArray<AFINNetworkAdapter*> Adapters = GetAttachedNetworkAdapters(Cast<AActor>(Obj));
		if (Adapters.Num() > 0) return Adapters[0]->Connector;
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

	TArray<AFINNetworkAdapter*> adapters = GetAttachedNetworkAdapters(hit.GetActor());

	for (AFINNetworkAdapter* adapter : adapters) {
		FVector npos = adapter->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = adapter->Connector;
		}
	}

	return connector;
}
