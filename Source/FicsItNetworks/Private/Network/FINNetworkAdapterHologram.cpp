#include "Network/FINNetworkAdapterHologram.h"
#include "Network/FINNetworkAdapter.h"
#include "FGOutlineComponent.h"

AFINNetworkAdapterHologram::AFINNetworkAdapterHologram() {
	this->mNeedsValidFloor = false;
	SetSnapToGuideLines(false);
}

USceneComponent* AFINNetworkAdapterHologram::SetupComponent(USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName) {
	USceneComponent* Component = Super::SetupComponent(attachParent, componentTemplate, componentName);
	if (componentTemplate->IsA<UStaticMeshComponent>()) CachedComponents.Add(Component);
	return Component;
}

bool AFINNetworkAdapterHologram::IsValidHitResult(const FHitResult& hitResult) const {
	return hitResult.Actor.IsValid();
}

void AFINNetworkAdapterHologram::OnInvalidHitResult() {
	UpdateSnapped(nullptr);
}

bool AFINNetworkAdapterHologram::TrySnapToActor(const FHitResult& hitResult) {
	bSnapped = false;
	bNeedsMesh = false;
	
	AActor* actor = hitResult.GetActor();
	if (!actor) {
		OnInvalidHitResult();
		return false;
	}
	
	// Try find SceneComponent that can be used a network connection
	if (AFINNetworkAdapter::FindConnection(actor, hitResult.Location, SnappedTransform, bNeedsMesh, MaxCables)) {
		bSnapped = true;
		SetHologramLocationAndRotation(hitResult);
		return true;
	}

	OnInvalidHitResult();
	return false;
}

void AFINNetworkAdapterHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	if (bSnapped) {
		for (USceneComponent* Component : CachedComponents) Cast<UStaticMeshComponent>(Component)->SetVisibility(bNeedsMesh);
		SetActorTransform(SnappedTransform);
		UpdateSnapped(hitResult.GetActor());
	} else {
		OnInvalidHitResult();
	}
}

void AFINNetworkAdapterHologram::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	AFINNetworkAdapter* Adapter = Cast<AFINNetworkAdapter>(inBuildable);
	if (Adapter && bSnapped) {
		Adapter->Parent = Cast<AFGBuildable>(PrevSnappedActor);
	}
}

void AFINNetworkAdapterHologram::UpdateSnapped(AActor* NewSnappedActor) {
	if (PrevSnappedActor != NewSnappedActor) {
		OnEndSnap(PrevSnappedActor);
		OnBeginSnap(NewSnappedActor);
		PrevSnappedActor = NewSnappedActor;
	}
}

void AFINNetworkAdapterHologram::OnBeginSnap(AActor* ActorSnapped) {
	if (ActorSnapped) {
		UFGOutlineComponent* Outline = UFGOutlineComponent::Get(this->GetWorld());
		if (Outline) Outline->ShowOutline(ActorSnapped, EOutlineColor::OC_HOLOGRAM);
	}
}

void AFINNetworkAdapterHologram::OnEndSnap(AActor* ActorUnsapped) {
	if (ActorUnsapped) {
		UFGOutlineComponent* Outline = UFGOutlineComponent::Get(this->GetWorld());
		if (Outline) Outline->HideOutline();
	}
}
