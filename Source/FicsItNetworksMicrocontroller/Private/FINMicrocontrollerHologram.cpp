#include "FINMicrocontrollerHologram.h"

#include "FGConstructDisqualifier.h"
#include "FGOutlineComponent.h"
#include "FINMicrocontroller.h"
#include "FINMicrocontrollerReference.h"
#include "FINNetworkUtils.h"

void AFINMicrocontrollerHologram::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (SnappedNetworkComponent) {
		UFGOutlineComponent* Outline = UFGOutlineComponent::Get(GetWorld());
		if (Outline) Outline->HideOutline(SnappedNetworkComponent);
	}

	Super::EndPlay(EndPlayReason);
}

bool AFINMicrocontrollerHologram::IsValidHitActor(AActor* hitActor) const {
	return UFINNetworkUtils::FindNetworkComponentFromObject(hitActor) != nullptr;
}

inline void AFINMicrocontrollerHologram::OnInvalidHitResult() {
	Super::OnInvalidHitResult();

	if (SnappedNetworkComponent) {
		UFGOutlineComponent* Outline = UFGOutlineComponent::Get(GetWorld());
		if (Outline) Outline->HideOutline(SnappedNetworkComponent);
	}
	SnappedNetworkComponent = nullptr;
}

bool AFINMicrocontrollerHologram::TrySnapToActor(const FHitResult& hitResult) {

	if (UFINNetworkUtils::FindNetworkComponentFromObject(hitResult.GetActor())) {
		if (SnappedNetworkComponent != hitResult.GetActor()) {
			if (SnappedNetworkComponent) {
				UFGOutlineComponent* Outline = UFGOutlineComponent::Get(GetWorld());
				if (Outline) Outline->HideOutline(SnappedNetworkComponent);
			}

			SnappedNetworkComponent = hitResult.GetActor();

			OnSnap();
			bool bExistingMicrocontroller = UFINMicrocontrollerReference::FindMicrocontroller(SnappedNetworkComponent) != nullptr;
			UFGOutlineComponent* Outline = UFGOutlineComponent::Get(GetWorld());
			if (Outline) Outline->ShowOutline(SnappedNetworkComponent, bExistingMicrocontroller ? EOutlineColor::OC_RED : EOutlineColor::OC_HOLOGRAMLINE);
		}
		return true;
	}

	if (SnappedNetworkComponent) {
		UFGOutlineComponent* Outline = UFGOutlineComponent::Get(GetWorld());
		if (Outline) Outline->HideOutline(SnappedNetworkComponent);
	}
	SnappedNetworkComponent = nullptr;
	return false;
}

void AFINMicrocontrollerHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {

	if (SnappedNetworkComponent) {
		SetActorLocation(SnappedNetworkComponent->GetActorLocation());
	} else {
		Super::SetHologramLocationAndRotation(hitResult);
	}
}

void AFINMicrocontrollerHologram::CheckValidPlacement() {
	if (!SnappedNetworkComponent) {

		Super::CheckValidPlacement();
	}
	if (UFINMicrocontrollerReference::FindMicrocontroller(SnappedNetworkComponent)) {
		AddConstructDisqualifier(UFINCDExistingMicrocontroller::StaticClass());
	}
}

void AFINMicrocontrollerHologram::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	AFINMicrocontroller* controller = Cast<AFINMicrocontroller>(inBuildable);
	controller->NetworkComponent = SnappedNetworkComponent;
}
