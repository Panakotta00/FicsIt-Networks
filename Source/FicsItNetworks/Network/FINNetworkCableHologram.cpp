#include "FINNetworkCableHologram.h"


#include "FGConstructDisqualifier.h"
#include "FINNetworkAdapter.h"
#include "FINComponentUtility.h"

#include "FGPowerConnectionComponent.h"

#include "FINNetworkCable.h"
#include "UnrealNetwork.h"

FVector FFINSnappedInfo::GetConnectorPos() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
	case FIN_POWER:
		return Cast<USceneComponent>(SnappedObj)->GetComponentLocation();
	case FIN_SETTINGS:
		return Cast<AFGBuildable>(SnappedObj)->GetActorTransform().TransformPosition(AdapterSettings.loc);
	case FIN_POLE: {
		return PolePostition + FVector(0,0,635);
	} default:
		return FVector::ZeroVector;
	}
}

FRotator FFINSnappedInfo::GetConnectorRot() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
	case FIN_POWER: {
		USceneComponent* Component = Cast<USceneComponent>(SnappedObj);
		return Component->GetComponentRotation();
	} case FIN_SETTINGS:
		return Cast<AFGBuildable>(SnappedObj)->GetActorTransform().TransformRotation(AdapterSettings.rot.Quaternion()).Rotator();
	case FIN_POLE: {
		AFINNetworkCableHologram* Holo = Cast<AFINNetworkCableHologram>(SnappedObj);
		check(Holo);
		return Holo->GetActorRotation();
	} default:
		return FRotator::ZeroRotator;
	}
}

AActor* FFINSnappedInfo::GetActor() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
		return Cast<UFINNetworkConnectionComponent>(SnappedObj)->GetAttachmentRootActor();
	case FIN_SETTINGS:
		return Cast<AFGBuildable>(SnappedObj);
	case FIN_POWER:
		return Cast<UFGPowerConnectionComponent>(SnappedObj)->GetAttachmentRootActor();
	default:
		return nullptr;
	}
}

void AFINNetworkCableHologram::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINNetworkCableHologram, Snapped);
	DOREPLIFETIME(AFINNetworkCableHologram, OldSnapped);
	DOREPLIFETIME(AFINNetworkCableHologram, From);
}

bool AFINNetworkCableHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if (From.SnapType != FIN_NOT_SNAPPED) return IsSnappedValid();
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		return false;
	}
	if (!IsSnappedValid()) return false;

	From = Snapped;
	Snapped = {FIN_NOT_SNAPPED};

	return false;
}

UFINNetworkConnectionComponent* AFINNetworkCableHologram::SetupSnapped(FFINSnappedInfo s, TArray<AActor*>& childs, FNetConstructionID id) {
	switch (s.SnapType) {
	case FIN_CONNECTOR:
		return Cast<UFINNetworkConnectionComponent>(s.SnappedObj);
	case FIN_SETTINGS:
	case FIN_POWER: {
		AFGBuildable* Buildable = Cast<AFGBuildable>(s.GetActor());
		FRotator Rotation = s.GetConnectorRot() + FRotator(0,90,0);
		FVector Location = s.GetConnectorPos();
		FActorSpawnParameters spawnParams;
		spawnParams.bDeferConstruction = true;
		AFINNetworkAdapter* a = GetWorld()->SpawnActor<AFINNetworkAdapter>(Location, Rotation, spawnParams);
		a->Parent = Buildable;
		UGameplayStatics::FinishSpawningActor(a, a->GetTransform());
		a->SetActorRotation(Rotation);
		a->SetActorLocation(Location);
		return a->Connector;
	} case FIN_POLE: {
		AActor* Constructed = PoleHologram->Construct(childs, id);
		childs.Add(Constructed);
		TArray<UActorComponent*> Comp = Constructed->GetComponentsByClass(UFINNetworkConnectionComponent::StaticClass());
		check(Comp.Num() > 0);
		return Cast<UFINNetworkConnectionComponent>(Comp[0]);
	} default:
		return nullptr;
	}
}

AActor* AFINNetworkCableHologram::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	UFINNetworkConnectionComponent* c1 = SetupSnapped(Snapped, childs, constructionID);
	UFINNetworkConnectionComponent* c2 = SetupSnapped(From, childs, constructionID);
	
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
	if (Snapped.SnapType == FIN_NOT_SNAPPED || From.SnapType == FIN_NOT_SNAPPED) return 0.0;
	return (Snapped.GetConnectorPos() - From.GetConnectorPos()).Size()/1000.0;
}

bool AFINNetworkCableHologram::IsValidHitResult(const FHitResult& hit) const {
	return hit.Actor.IsValid();
}

bool AFINNetworkCableHologram::TrySnapToActor(const FHitResult& hitResult) {
	// Check if hit actor is valid
	auto actor = hitResult.Actor.Get();
	if (!actor) {
		Snapped = {FIN_NOT_SNAPPED};
		SetHologramLocationAndRotation(hitResult);
		OnInvalidHitResult();
		return false;
	}
		
	// Try to find network connector
	UFINNetworkConnectionComponent* Connector = UFINComponentUtility::GetNetworkConnectorFromHit(hitResult);
	if (Connector) {
		// Use networks connector as snapping point
		Snapped = {FIN_CONNECTOR, Connector};
		SetHologramLocationAndRotation(hitResult);
		return true;
	}

	// find the nearest power connector to hit if actor is factory
	if (actor->IsA<AFGBuildable>()) {
		const TArray<UActorComponent*> cons = actor->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
		float dist = -1.0f;
		USceneComponent* con = nullptr;
		for (UActorComponent* c : cons) {
			float d = (Cast<USceneComponent>(c)->GetComponentToWorld().GetTranslation() - hitResult.Location).Size();
			if (dist < 0.0f || dist > d) {
				con = Cast<USceneComponent>(c);
				dist = d;
			}
		}
		if (con) {
			// use nearest power connector as connection point by using adapter logic
			Snapped = {FIN_POWER, con};
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	// find pre defined adapter setting
	for (auto entry : AFINNetworkAdapter::settings) {
		auto setting = entry.Value;
		auto clazz = entry.Key;

		if (actor->IsA(clazz)) {
			auto t = actor->GetTransform().TransformPosition(setting.loc);
			auto r = actor->GetTransform().TransformRotation(setting.rot.Quaternion());
			Snapped = { FIN_SETTINGS, actor, FVector(), setting };
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	// no connection point found -> pole
	if (From.SnapType != FIN_NOT_SNAPPED) {
		Snapped = {FIN_POLE, this, hitResult.Location};
	} else Snapped = {FIN_NOT_SNAPPED, nullptr};

	SetHologramLocationAndRotation(hitResult);
	
	return false;
}

bool AFINNetworkCableHologram::IsSnappedValid() {
	bool ret = true;
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
		ret = false;
	}
	if (Snapped.SnapType == FIN_CONNECTOR) {
		UFINNetworkConnectionComponent* Connector = Cast<UFINNetworkConnectionComponent>(Snapped.SnappedObj);
		if (Connector->ConnectedCables.Num() >= Connector->MaxCables) {
			AddConstructDisqualifier(UFGCDWireTooManyConnections::StaticClass());
			ret = false;
		}
		if (From.SnapType == FIN_CONNECTOR) {
			UFINNetworkConnectionComponent* FromConnector = Cast<UFINNetworkConnectionComponent>(From.SnappedObj);
			for (AFINNetworkCable* Cable : Connector->ConnectedCables) {
				if (Cable->Connector1 == FromConnector || Cable->Connector2 == FromConnector) {
					AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
					ret = false;
				}
            }
		}
	}
	if (From.SnapType != FIN_NOT_SNAPPED && Snapped.SnapType != FIN_NOT_SNAPPED && (From.GetConnectorPos() - Snapped.GetConnectorPos()).Size() > 10000.0f) {
		AddConstructDisqualifier(UFGCDWireTooLong::StaticClass());
		ret = false;
	}
	return ret;
}

void AFINNetworkCableHologram::SetHologramLocationAndRotation(const FHitResult& hit) {
	if (From.SnapType == FIN_NOT_SNAPPED) this->RootComponent->SetWorldLocation(Snapped.GetConnectorPos());
	else this->RootComponent->SetWorldLocation(From.GetConnectorPos());

	UpdateSnapped();
	bool validSnap = IsSnappedValid();
	
	if (Snapped.SnapType == FIN_NOT_SNAPPED || From.SnapType == FIN_NOT_SNAPPED) {
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
	FVector end = RootComponent->GetComponentToWorld().InverseTransformPosition(Snapped.GetConnectorPos());
	FVector start_t = end;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;
	
	Cable->SetStartAndEnd(start, start_t, end, end_t, true);

	if (From.SnapType == FIN_SETTINGS) {
		Adapter1->SetVisibilitySML(true, true);
		Adapter1->SetRelativeRotation(RootComponent->GetComponentToWorld().InverseTransformRotation(From.GetConnectorRot().Quaternion()).Rotator());
	} else {
		Adapter1->SetVisibilitySML(false, true);
	}
	
	if (Snapped.SnapType == FIN_SETTINGS) {
		Adapter2->SetVisibilitySML(true, true);
		Adapter2->SetRelativeLocation(end);
		Adapter2->SetWorldRotation(Snapped.GetConnectorRot());
	} else {
		Adapter2->SetVisibilitySML(false, true);
	}

	PoleHologram->SetScrollRotateValue(GetScrollRotateValue());
	PoleHologram->SetHologramLocationAndRotation(hit);

	if (validSnap) {
		Cable->SetMaterial(0, mValidPlacementMaterial);
		Adapter1->SetMaterial(0, mValidPlacementMaterial);
		Adapter2->SetMaterial(0, mValidPlacementMaterial);
		for (UActorComponent* Comp : PoleHologram->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, mValidPlacementMaterial);
	} else {
		Cable->SetMaterial(0, mInvalidPlacementMaterial);
		Adapter1->SetMaterial(0, mInvalidPlacementMaterial);
		Adapter2->SetMaterial(0, mInvalidPlacementMaterial);
		for (UActorComponent* Comp : PoleHologram->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, mInvalidPlacementMaterial);
	}
}

bool AFINNetworkCableHologram::IsChanged() const {
	return Snapped.SnapType != FIN_NOT_SNAPPED;
}

USceneComponent* AFINNetworkCableHologram::SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) {
	return nullptr;
}

void AFINNetworkCableHologram::SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
	TSubclassOf<UFGRecipe> Recipe = LoadObject<UClass>(NULL, TEXT("/Game/FicsItNetworks/Network/NetworkPole/Recipe_NetworkPole.Recipe_NetworkPole_C"));
	PoleHologram = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, Recipe, hologramOwner, spawnLocation, hologramInstigator));
	PoleHologram->SetDisabled(true);
}

void AFINNetworkCableHologram::OnInvalidHitResult() {
	OnEndSnap(Snapped);
	OnEndSnap(OldSnapped);
	Snapped.SnapType = OldSnapped.SnapType = FIN_NOT_SNAPPED;
}

void AFINNetworkCableHologram::UpdateSnapped() {
	if (Snapped.SnappedObj != OldSnapped.SnappedObj || Snapped.SnapType != OldSnapped.SnapType) {
		OnEndSnap(OldSnapped);
		OnBeginSnap(Snapped, IsSnappedValid());

		if (Snapped.SnapType == FIN_POLE) {
			PoleHologram->SetDisabled(false);
		} else {
			PoleHologram->SetDisabled(true);
		}

		OldSnapped = Snapped;
	}
}

void AFINNetworkCableHologram::OnBeginSnap(FFINSnappedInfo a, bool isValid) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) UFGOutlineComponent::Get(this->GetWorld())->ShowOutline(o, isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED);
		// TODO: Do snap sound
		//this->Client_PlaySnapSound();
	}
}

void AFINNetworkCableHologram::OnEndSnap(FFINSnappedInfo a) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) UFGOutlineComponent::Get(o->GetWorld())->HideOutline();
	}
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
	Adapter1->SetVisibilitySML(false);
	Adapter1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Adapter2 = CreateDefaultSubobject<UStaticMeshComponent>(L"Adapter2");
	Adapter2->SetupAttachment(RootComponent);
	Adapter2->SetMobility(EComponentMobility::Movable);
	Adapter2->SetStaticMesh(adapterMesh);
	Adapter2->SetVisibilitySML(false);
	Adapter2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AFINNetworkCableHologram::~AFINNetworkCableHologram() {}
