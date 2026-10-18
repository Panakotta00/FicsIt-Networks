#include "FINNetworkUtils.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkComponent.h"
#include "EngineUtils.h"

namespace {
	/**
	 * Collects the network adapters attached to the given actor.
	 * The adapter references only exist locally, on clients they can be missing (f.e. if the parent was not replicated yet when the adapter began play),
	 * so clients fall back to searching the adapters by their replicated parent.
	 */
	void GetAttachedAdapters(AActor* Actor, TArray<AFINNetworkAdapter*>& OutAdapters) {
		TArray<UFINNetworkAdapterReference*> AdapterRefs;
		Actor->GetComponents<UFINNetworkAdapterReference>(AdapterRefs);
		for (UFINNetworkAdapterReference* AdapterRef : AdapterRefs) {
			if (AdapterRef && IsValid(AdapterRef->Ref)) OutAdapters.AddUnique(AdapterRef->Ref);
		}

		if (OutAdapters.Num() > 0 || Actor->GetNetMode() != NM_Client) return;
		for (TActorIterator<AFINNetworkAdapter> It(Actor->GetWorld()); It; ++It) {
			if (It->Parent == Actor) OutAdapters.Add(*It);
		}
	}
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
		TArray<AFINNetworkAdapter*> Adapters;
		GetAttachedAdapters(Cast<AActor>(Obj), Adapters);
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

	TArray<AFINNetworkAdapter*> adapters;
	GetAttachedAdapters(hit.GetActor(), adapters);

	for (AFINNetworkAdapter* adapter : adapters) {
		FVector npos = adapter->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = adapter->Connector;
		}
	}

	return connector;
}
