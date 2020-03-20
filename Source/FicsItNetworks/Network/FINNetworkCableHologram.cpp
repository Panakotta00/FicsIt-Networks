#include "FINNetworkCableHologram.h"

#include "FINNetworkAdapter.h"
#include "FINComponentUtility.h"

#include "FGPlayerController.h"
#include "FGPowerConnectionComponent.h"

#include "Kismet/KismetMathLibrary.h"

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
	UGameplayStatics::FinishSpawningActor(a, s.f()->GetTransform());
	a->SetActorRotation(rotation);
	a->SetActorLocation(location);
	return a->Connector;
}


AActor* AFINNetworkCableHologram::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	auto c1 = setupSnapped(Snapped);
	auto c2 = setupSnapped(From);
	
	FRotator rotation = FRotator();
	FVector location = c1->GetComponentToWorld().GetTranslation();
	FVector scale;
	scale.X = scale.Y = scale.Z = 1.0;
	FQuat rot;
	rot.W = 0;
	rot.X = 0;
	rot.Y = 0;
	rot.Z = 0;

	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;

	auto a = GetWorld()->SpawnActor<AFINNetworkCable>(this->mBuildClass, location, rotation, spawnParams);
	
	FTransform t = a->GetTransform();
	t.SetRotation(rot);
	t.SetScale3D(scale);
	t.SetTranslation(location);

	((UGameplayStatics*)UGameplayStatics::StaticClass()->GetDefaultObject())->FinishSpawningActor(a, t);

	return a;
}

int32 AFINNetworkCableHologram::GetBaseCostMultiplier() const {
	return (Snapped.pos - From.pos).Size();
}

bool AFINNetworkCableHologram::IsValidHitResult(const FHitResult& hit) const {
	auto actor = hit.Actor.Get();
	if (IsValid(actor)
		&& (
			IsValid(actor->GetComponentByClass(UFINNetworkConnector::StaticClass()))
			|| (
				actor->IsA<AFGBuildableFactory>()
				&&
				IsValid(actor->GetComponentByClass(UFGPowerConnectionComponent::StaticClass()))
			)
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
	if (actor->IsA<AFGBuildableFactory>()) {
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
			Snapped = {true, false, con, actor, con->GetComponentToWorld().GetTranslation()};
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	// find pre defined adapter setting
	for (auto entry : AFINNetworkAdapter::settings) {
		auto setting = entry.second;
		auto clazz = entry.first;

		if (actor->IsA(clazz)) {
			auto t = UKismetMathLibrary::TransformLocation(actor->GetTransform(), setting.loc);
			Snapped = { true, false, actor, actor, t };
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

void AFINNetworkCableHologram::OnInvalidHitResult() {
	onEndSnap(Snapped);
	onEndSnap(OldSnapped);
	Snapped.v = OldSnapped.v = false;
	Snapped.ptr = OldSnapped.ptr = nullptr;
}

void AFINNetworkCableHologram::onBeginSnap(FFINSnappedInfo a, bool isValid) {
	if (a.v) {
		auto o = a.actor;
		// TODO: Do outline
		//if (o) Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->->ShowOutline(o, isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED);
		//this->Client_PlaySnapSound();
	}
}

void AFINNetworkCableHologram::onEndSnap(FFINSnappedInfo a) {
	if (a.v) {
		auto o = a.actor;
		// TODO: do outline
		//if (o) Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->GetOutline()->HideOutline();
	}
}

void AFINNetworkCableHologram::updateMeshes() {
	if (!Snapped.v || !From.v || Snapped.ptr == From.ptr) {
		Cable->SetVisibility(false, true);
		if (mInvalidPlacementMaterial) Con->SetMaterial(0, mInvalidPlacementMaterial);
		return;
	}
	Cable->SetVisibility(true, true);

	float offset = 250.0;
	FVector start;
	start.X = start.Y = start.Z = 0;
	FVector end = UKismetMathLibrary::InverseTransformLocation(RootComponent->GetComponentToWorld(), From.pos);
	FVector start_t = end;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;
	
	Cable->SetStartAndEnd(start, start_t, end, end_t, true);

	if (isValid() && isSnappedValid()) {
		Cable->SetMaterial(0, mValidPlacementMaterial);
		Con->SetMaterial(0, mValidPlacementMaterial);
	} else {
		Cable->SetMaterial(0, mInvalidPlacementMaterial);
		Con->SetMaterial(0, mInvalidPlacementMaterial);
	}
}

bool AFINNetworkCableHologram::isValid() {
	if (!From.v || !Snapped.v) return false;
	TArray<AActor*> overlaps;
	Cable->GetOverlappingActors(overlaps);
	for (auto actor : overlaps) {
		if (From.actor != actor && (!From.isConnector ||From.c()->GetOuter() != actor) && Snapped.actor != actor && (!Snapped.isConnector || Snapped.c()->GetOuter() != actor) && this != actor) {
			return false;
		}
	}
	return ((From.isConnector && Snapped.isConnector) ? !From.c()->IsConnected(Snapped.c()) : true) && (From.pos - Snapped.pos).Size() <= 10000.0f;
}

AFINNetworkCableHologram::AFINNetworkCableHologram() {
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> cableMesh(TEXT("StaticMesh'/Game/FicsItNetworks/Network/Mesh_Cable.Mesh_Cable"));
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> adapterMesh(TEXT("StaticMesh'/Game/FicsItNetworks/Network/Mesh_Adapter.Mesh_Adapter"));

	this->mMaxPlacementFloorAngle = 90.0f;

	Cable = CreateDefaultSubobject<USplineMeshComponent>(L"Cable");
	Cable->SetMobility(EComponentMobility::Movable);
	Cable->SetupAttachment(RootComponent);
	//Cable->SetStaticMesh(cableMesh.Object);
	Cable->ForwardAxis = ESplineMeshAxis::Z;
	Cable->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Cable->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	Cable->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	Cable->SetAllUseCCD(true);

	FVector loc;
	loc.X = loc.Y = loc.Z = 0.0;
	FVector scl;
	scl.X = scl.Y = scl.Z = 1.0;
	Con = CreateDefaultSubobject<USplineMeshComponent>(L"Adapter");
	Con->SetMobility(EComponentMobility::Movable);
	Con->SetupAttachment(RootComponent);
	//Con->SetStaticMesh(adapterMesh.Object);
	updateMeshes();
}

AFINNetworkCableHologram::~AFINNetworkCableHologram() {}
