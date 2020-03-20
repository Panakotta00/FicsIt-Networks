#include "FINNetworkCable.h"

#include "FINNetworkConnector.h"
#include "FINNetworkAdapter.h"

AFINNetworkCable::AFINNetworkCable() {}

AFINNetworkCable::~AFINNetworkCable() {}

void AFINNetworkCable::BeginPlay() {
	if (!IsValid(Connector1) || !IsValid(Connector2)) return;
	CableSpline->SetWorldLocation(Connector1->GetComponentLocation());
	
}

void AFINNetworkCable::Dismantle_Implementation() {
	TArray<UActorComponent*> comps = GetComponentsByClass(USceneComponent::StaticClass());
	for (auto comp : comps) {
		if (!comp) continue;
		if (auto connector = Cast<UFINNetworkConnector>(comp)) {
			for (auto cable : connector->Cables) {
				if (!cable) continue;
				cable->Dismantle();
				cable->Destroy();
			}
		} else if (auto adapter = Cast<UFINNetworkAdapterReference>(comp)) {
			for (auto cable : adapter->Ref->Connector->Cables) {
				if (!cable) continue;
				cable->Dismantle();
				cable->Destroy();
			}
			adapter->Ref->Destroy();
		}
	}
}

void AFINNetworkCable::GetDismantleRefund_Implementation(TArray<FInventoryStack>& refund) const {
	/*auto f = (UProperty*)((UObject*)self)->clazz->childs;
	while (f) {
		if (f->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty) {
			auto o = *f->getValue<UNetworkConnector*>(self);
			if (o && o->IsA((SDK::UClass*) UNetworkConnector::staticClass())) {
				for (auto c : o->cables) {
					auto f = ((UObject*)c)->findFunction(L"GetDismantleRefund");
					auto* r = &refund;
					f->invoke((UObject*)c, (SDK::TArray<SDK::FInventoryStack>*) r);
				}
			}
		}
		f = (UProperty*)f->next;
	}*/

	Super::GetDismantleRefund_Implementation( refund);
}