#include "FINNetworkUtils.h"

#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

UObject* UFINNetworkUtils::FindNetworkComponentFromObject(UObject* Obj) {
	if (Obj->Implements<UFINNetworkComponent>()) return Obj;
	if (Obj->IsA<AActor>()) {
		TArray<UActorComponent*> Connectors = Cast<AActor>(Obj)->GetComponentsByClass(UFINAdvancedNetworkConnectionComponent::StaticClass());
		for (UActorComponent* Connector : Connectors) {
			if (Connector->Implements<UFINNetworkComponent>()) return Connector;
		}
	}
	return nullptr;
}
