#include "FINNetworkCableHologram.h"

#include "FINNetworkAdapter.h"
#include "FINComponentUtility.h"

#include "FGPlayerController.h"
#include "FGPowerConnectionComponent.h"

bool AFINNetworkCableHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if (From.ptr == Snapped.ptr) return false;
	if (From.v && Snapped.v) return isValid() && isSnappedValid();
	if (!Snapped.v) {
		updateMeshes();
		return false;
	}
	if (!isSnappedValid()) return false;

	From = Snapped;

	onBeginSnap(From, false);

	return false;
}

UFINNetworkConnector* AFINNetworkCableHologram::setupSnapped(FFINSnappedInfo s) {
	if (s.isConnector) return s.c();
	
	FRotator rotation = s.f()->GetActorRotation();
	FVector location = s.pos;
	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;
	auto a = GetWorld()->SpawnActor<AFINNetworkAdapter>(location, rotation, spawnParams);
	a->Parent = s.f();
	UGameplayStatics::FinishSpawningActor(a, a->GetTransform());
	a->SetActorRotation(rotation);
	a->SetActorLocation(location);
	return a->Connector;
}


AActor* AFINNetworkCableHologram::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	auto c1 = setupSnapped(Snapped);
	auto c2 = setupSnapped(From);
	
	FRotator rotation = FRotator::ZeroRotator;
	FVector location = c1->GetComponentToWorld().GetTranslation();
	
	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;

	AFINNetworkCable* a = GetWorld()->SpawnActor<AFINNetworkCable>(this->mBuildClass, location, rotation, spawnParams);
	
	FTransform t = a->GetTransform();

	a->Connector1 = c1;
	a->Connector2 = c2;

	a->SetBuiltWithRecipe(GetRecipe());
	
	return UGameplayStatics::FinishSpawningActor(a, FTransform(rotation.Quaternion(), location));
}

int32 AFINNetworkCableHologram::GetBaseCostMultiplier() const {
	return (Snapped.pos - From.pos).Size()/100.0;
}

bool AFINNetworkCableHologram::IsValidHitResult(const FHitResult& hit) const {
	auto actor = hit.Actor.Get();
	if (!IsValid(actor)) return false;
	if (IsValid(actor->GetComponentByClass(UFINNetworkConnector::StaticClass()))
		|| (
			actor->IsA<AFGBuildable>()
			&&
			IsValid(actor->GetComponentByClass(UFGPowerConnectionComponent::StaticClass()))
		)) return true;
	for (auto entry : AFINNetworkAdapter::settings) {
		auto clazz = entry.first;

		if (actor->IsA(clazz)) return true;
	}
	return false;
}

bool AFINNetworkCableHologram::TrySnapToActor(const FHitResult& hitResult) {
	// Check if hit actor is valid
	auto actor = hitResult.Actor.Get();
	if (!IsValid(actor)) return false;

	// Try to find network connector
	auto c = UFINComponentUtility::GetNetworkConnectorFromHit(hitResult);
	if (c) {
		// Use networks connector as snapping point
		Snapped = {true, true, c, actor, c->GetComponentToWorld().GetTranslation()};
		SetHologramLocationAndRotation(hitResult);
		return true;
	}

	// find the nearest power connector to hit if actor is factory
	if (actor->IsA<AFGBuildable>()) {
		auto cons = actor->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
		float dist = -1.0f;
		USceneComponent* con = nullptr;
		for (auto c : cons) {
			float d = (Cast<USceneComponent>(c)->GetComponentToWorld().GetTranslation() - hitResult.Location).Size();
			if (dist < 0.0f || dist > d) {
				con = Cast<USceneComponent>(c);
				dist = d;
			}
		}
		if (con) {
			// use nearest power connector as connection point by using adapter logic
			Snapped = {true, false, actor, actor, con->GetComponentToWorld().GetTranslation()};
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	// find pre defined adapter setting
	for (auto entry : AFINNetworkAdapter::settings) {
		auto setting = entry.second;
		auto clazz = entry.first;

		if (actor->IsA(clazz)) {
			auto t = actor->GetTransform().TransformPosition(setting.loc);
			auto r = actor->GetTransform().TransformRotation(setting.rot.Quaternion());
			Snapped = { true, false, actor, actor, t, r, true };
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	// no connection point found
	Snapped.v = false;
	Snapped.ptr = nullptr;

	return false;
}

bool AFINNetworkCableHologram::isSnappedValid() {
	return Snapped.v && (!Snapped.isConnector || (Snapped.c()->Cables.Num() < Snapped.c()->MaxCables));
}

void AFINNetworkCableHologram::SetHologramLocationAndRotation(const FHitResult& hit) {
	if (!Snapped.v) return;

	this->RootComponent->SetWorldLocation((Snapped.v) ? Snapped.pos : hit.Location);

	updateMeshes();

	if (Snapped.ptr != OldSnapped.ptr) {
		onEndSnap(OldSnapped);
		onBeginSnap(Snapped, isSnappedValid() && (Snapped.ptr != From.ptr) && (!From.v || (isValid())));

		OldSnapped = Snapped;
	}
}

bool AFINNetworkCableHologram::IsChanged() const {
	return OldSnapped.v;
}

USceneComponent* AFINNetworkCableHologram::SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) {
	return nullptr;
}

void AFINNetworkCableHologram::OnInvalidHitResult() {
	onEndSnap(Snapped);
	onEndSnap(OldSnapped);
	Snapped.v = OldSnapped.v = false;
	Snapped.ptr = OldSnapped.ptr = nullptr;
}

void AFINNetworkCableHologram::onBeginSnap(FFINSnappedInfo a, bool isValid) {
	if (a.v) {
		auto o = a.actor;
		if (o) UFGOutlineComponent::Get(o->GetWorld())->ShowOutline(o, isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED);
		// TODO: Do snap sound
		//this->Client_PlaySnapSound();
	}
}

void AFINNetworkCableHologram::onEndSnap(FFINSnappedInfo a) {
	if (a.v) {
		auto o = a.actor;
		if (o) UFGOutlineComponent::Get(o->GetWorld())->HideOutline();
	}
}

void AFINNetworkCableHologram::updateMeshes() {
	if (!Snapped.v || !From.v || Snapped.ptr == From.ptr) {
		Cable->SetVisibilitySML(false, true);
		Adapter1->SetVisibilitySML(false, true);
		Adapter2->SetVisibilitySML(false, true);

		if (mInvalidPlacementMaterial) {
			Adapter1->SetMaterial(0, mInvalidPlacementMaterial);
			Adapter2->SetMaterial(0, mInvalidPlacementMaterial);
		}
		return;
	}
	Cable->SetVisibilitySML(true, true);

	float offset = 250.0;
	FVector start;
	start.X = start.Y = start.Z = 0;
	FVector end = RootComponent->GetComponentToWorld().InverseTransformPosition(From.pos);
	FVector start_t = end;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;
	
	Cable->SetStartAndEnd(start, start_t, end, end_t, true);

	if (!Snapped.isConnector && ((UObject*)Snapped.ptr)->IsA<AActor>() && Snapped.setting) {
		Adapter1->SetVisibilitySML(true, true);
		Adapter1->SetRelativeRotation(Snapped.rot);
	}
	
	if (!From.isConnector && ((UObject*)From.ptr)->IsA<AActor>() && From.setting) {
		Adapter2->SetVisibilitySML(true, true);
		Adapter2->SetRelativeLocation(end);
		Adapter2->SetRelativeRotation(From.rot);
	}

	if (isValid() && isSnappedValid()) {
		Cable->SetMaterial(0, mValidPlacementMaterial);
		Adapter1->SetMaterial(0, mValidPlacementMaterial);
		Adapter2->SetMaterial(0, mValidPlacementMaterial);
	} else {
		Cable->SetMaterial(0, mInvalidPlacementMaterial);
		Adapter1->SetMaterial(0, mInvalidPlacementMaterial);
		Adapter2->SetMaterial(0, mInvalidPlacementMaterial);
	}}

bool AFINNetworkCableHologram::isValid() {
	if (!From.v || !Snapped.v) return false;
	TArray<AActor*> overlaps;
	Cable->GetOverlappingActors(overlaps);
	for (auto actor : overlaps) {
		if (From.actor != actor && (!From.isConnector ||From.c()->GetOuter() != actor) && Snapped.actor != actor && (!Snapped.isConnector || Snapped.c()->GetOuter() != actor) && this != actor && !actor->IsA<AFINNetworkCable>()) {
			return false;
		}
	}
	return ((From.isConnector && Snapped.isConnector) ? !From.c()->IsConnected(Snapped.c()) : true) && (From.pos - Snapped.pos).Size() <= 10000.0f;
}

AFINNetworkCableHologram::AFINNetworkCableHologram() {
	UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable"));
	UStaticMesh* adapterMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/Mesh_Adapter.Mesh_Adapter"));

	this->mMaxPlacementFloorAngle = 90.0f;

	RootComponent->SetWorldScale3D(FVector::OneVector);

	Cable = CreateDefaultSubobject<USplineMeshComponent>(L"Cable");
	Cable->SetMobility(EComponentMobility::Movable);
	Cable->SetupAttachment(RootComponent);
	Cable->SetStaticMesh(cableMesh);
	Cable->ForwardAxis = ESplineMeshAxis::Z;
	Cable->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Cable->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Cable->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cable->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
	Cable->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
	Cable->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Block);
	Cable->SetAllUseCCD(true);

	Adapter1 = CreateDefaultSubobject<UStaticMeshComponent>(L"Adapter1");
	Adapter1->SetupAttachment(RootComponent);
	Adapter1->SetMobility(EComponentMobility::Movable);
	Adapter1->SetStaticMesh(adapterMesh);

	Adapter2 = CreateDefaultSubobject<UStaticMeshComponent>(L"Adapter2");
	Adapter2->SetupAttachment(RootComponent);
	Adapter2->SetMobility(EComponentMobility::Movable);
	Adapter2->SetStaticMesh(adapterMesh);

	updateMeshes();
}

AFINNetworkCableHologram::~AFINNetworkCableHologram() {}
