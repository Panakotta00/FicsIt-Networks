#include "FINNetworkUtils.h"


#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkComponent.h"
#include "Components/ActorComponent.h"

UObject* UFINNetworkUtils::FindNetworkComponentFromObject(UObject* Obj) {
	if (Obj->IsA<UFINNetworkComponent>()) return Obj;
	if (Obj->IsA<AActor>()) {
		TArray<UActorComponent*> Connectors;
		Cast<AActor>(Obj)->GetComponents(Connectors, UFINAdvancedNetworkConnectionComponent::StaticClass());
		for (UActorComponent* Connector : Connectors) {
			return Connector;
		}
	}
	return nullptr;
}
