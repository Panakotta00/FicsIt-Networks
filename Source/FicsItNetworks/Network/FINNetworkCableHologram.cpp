#include "FINNetworkCableHologram.h"

#include "FGBackgroundThread.h"
#include "FGConstructDisqualifier.h"
#include "FGOutlineComponent.h"
#include "FINNetworkAdapter.h"
#include "FGPowerConnectionComponent.h"
#include "FINNetworkCable.h"
#include "FicsItNetworks/FINComponentUtility.h"
#include "FicsItNetworks/Utils/FINWallAndFoundationHologram.h"

#pragma optimize("", off)
FVector FFINCablePlacementStepInfo::GetConnectorPos() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
	case FIN_POWER:
		return Cast<USceneComponent>(SnappedObj)->GetComponentLocation();
	case FIN_SETTINGS:
		return Cast<AFGBuildable>(SnappedObj)->GetActorTransform().TransformPosition(AdapterSettings.loc);
	case FIN_POLE: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		AFGBuildable* Pole = AFINNetworkCableHologram::GetDefaultBuildable_Static<AFGBuildable>(Holo);
		return Cast<AActor>(SnappedObj)->GetActorLocation() + FVector(0,0,635);
	}
	case FIN_PLUG: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		TArray<UFINNetworkConnectionComponent*> Connector = UFINComponentUtility::GetComponentsFromSubclass<UFINNetworkConnectionComponent>(AFINNetworkCableHologram::GetBuildClass(Holo));
		if (Connector.Num() > 0) return Holo->GetTransform().TransformPosition(Connector[0]->GetRelativeLocation());
		else return Holo->GetActorLocation();
	} default:
		return FVector::ZeroVector;
	}
}
#pragma optimize("", on)

FRotator FFINCablePlacementStepInfo::GetConnectorRot() const {
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
	} case FIN_PLUG: {
		AFINWallAndFoundationHologram* Holo2 = Cast<AFINWallAndFoundationHologram>(SnappedObj);
		check(Holo2);
		return Holo2->GetActorRotation();
	} default:
		return FRotator::ZeroRotator;
	}
}

AActor* FFINCablePlacementStepInfo::GetActor() const {
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

void AFINNetworkCableHologram::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	UStaticMesh* Mesh = GetDefaultBuildable<AFINNetworkCable>()->CableSpline->GetStaticMesh();
	Cable->SetStaticMesh(Mesh);
}

bool AFINNetworkCableHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if (IsInSecondStep()) return IsSnappedValid();
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		return false;
	}
	if (!IsSnappedValid()) return false;

	From = Snapped;
	Snapped = {FIN_NOT_SNAPPED};

	return false;
}

UFINNetworkConnectionComponent* AFINNetworkCableHologram::SetupSnapped(FFINCablePlacementStepInfo s) {
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
		return nullptr;
	} case FIN_PLUG: {
		return nullptr;
	} default:
		return nullptr;
	}
}

AActor* AFINNetworkCableHologram::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	Connector1Cache = SetupSnapped(From);
	Connector2Cache = SetupSnapped(Snapped);

	AFINNetworkCable* FinishedCable = Cast<AFINNetworkCable>(Super::Construct(childs, constructionID));
	int i = childs.Num()-1;
	if (From.SnapType == FIN_POLE || From.SnapType == FIN_PLUG) {
		AActor* Plug = childs[i--];
		UFINNetworkConnectionComponent* Con = Cast<UFINNetworkConnectionComponent>(Plug->GetComponentByClass(UFINNetworkConnectionComponent::StaticClass()));
		Connector1Cache = Con;
	}
	if (Snapped.SnapType == FIN_POLE || Snapped.SnapType == FIN_PLUG) {
		AActor* Pole = childs[i--];
		UFINNetworkConnectionComponent* Con = Cast<UFINNetworkConnectionComponent>(Pole->GetComponentByClass(UFINNetworkConnectionComponent::StaticClass()));
		Connector2Cache = Con;
	}
	if (Connector1Cache) FinishedCable->SetActorLocation(Connector1Cache->GetComponentToWorld().GetTranslation());

	FinishedCable->Connector1 = Connector1Cache;
	FinishedCable->Connector2 = Connector2Cache;
	FinishedCable->ConnectConnectors();
	FinishedCable->RerunConstructionScripts();
	
	Snapped = FFINCablePlacementStepInfo();
	From = FFINCablePlacementStepInfo();
	ForceNetUpdate();
	return FinishedCable;
}

void AFINNetworkCableHologram::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);
}

int32 AFINNetworkCableHologram::GetBaseCostMultiplier() const {
	if (Snapped.SnapType == FIN_NOT_SNAPPED || From.SnapType == FIN_NOT_SNAPPED) return 0.0;
	return (Snapped.GetConnectorPos() - From.GetConnectorPos()).Size()/1000.0;
}

bool AFINNetworkCableHologram::IsValidHitResult(const FHitResult& hit) const {
	return hit.Actor.IsValid();
}

bool AFINNetworkCableHologram::TrySnapToActor(const FHitResult& hitResult) {
	UStaticMeshComponent* Adapter = IsInSecondStep() ? Adapter1 : Adapter2;
	AFGBuildableHologram* PlugHolo = IsInSecondStep() ? PlugHologram1 : PlugHologram2;
	AFGBuildableHologram* PoleHolo = IsInSecondStep() ? PoleHologram1 : PoleHologram2;
	Adapter->SetVisibility(false, true);
	PoleHolo->SetDisabled(true);
	PlugHolo->SetDisabled(true);
	
	// Check if hit actor is valid
	auto actor = hitResult.Actor.Get();
	if (!actor) {
		Snapped = {FIN_NOT_SNAPPED};
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
			Snapped = { FIN_SETTINGS, actor, setting };
			SetHologramLocationAndRotation(hitResult);
			return true;
		}
	}

	Snapped = { FIN_NOT_SNAPPED };

	return false;
}

bool AFINNetworkCableHologram::IsSnappedValid() {
	bool ret = true;
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
		ret = false;
	}
	if (Snapped.SnappedObj == From.SnappedObj) {
		AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
		ret = false;
	}
	if (Snapped.SnapType == FIN_CONNECTOR) {
		UFINNetworkConnectionComponent* Connector = Cast<UFINNetworkConnectionComponent>(Snapped.SnappedObj);
		if (Connector->ConnectedCables.Num() >= Connector->MaxCables) {
			AddConstructDisqualifier(UFGCDWireTooManyConnections::StaticClass());
			ret = false;
		}
		if (!Connector->AllowedCableConnections.Contains(GetItemDescriptor())) {
			AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
			ret = false;
		}
		if (From.SnapType == FIN_CONNECTOR) {
			UFINNetworkConnectionComponent* FromConnector = Cast<UFINNetworkConnectionComponent>(From.SnappedObj);
			for (AFINNetworkCable* FCable : Connector->ConnectedCables) {
				if (FCable->Connector1 == FromConnector || FCable->Connector2 == FromConnector) {
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
	UStaticMeshComponent* Adapter = IsInSecondStep() ? Adapter1 : Adapter2;
	AFGBuildableHologram* PlugHolo = IsInSecondStep() ? PlugHologram1 : PlugHologram2;
	AFGBuildableHologram* PoleHolo = IsInSecondStep() ? PoleHologram1 : PoleHologram2;
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		AActor* Actor = hit.GetActor();
		if (Actor) {
			if (bEnablePlug && (Actor->IsA<AFGBuildableFoundation>() || Actor->IsA<AFGBuildableWall>()) && !hit.Normal.Equals(FVector(0, 0, 1), 0.1)) {
				Snapped = { FIN_PLUG, PlugHolo };
				PlugHolo->SetHologramLocationAndRotation(hit);
			} else if (bEnablePole) {
				Snapped = { FIN_POLE, PoleHolo };
				PoleHolo->SetHologramLocationAndRotation(hit);
			} else {
				Snapped = {FIN_NOT_SNAPPED, nullptr};
			}
		}
	}
	
	// place to snap start
	if (!IsInSecondStep()) this->RootComponent->SetWorldLocation(Snapped.GetConnectorPos());
	else this->RootComponent->SetWorldLocation(From.GetConnectorPos());

	// update snapp data and sub holos
	UpdateSnapped();
	
	bool validSnap = IsSnappedValid();
	UpdateMeshValidity(validSnap);

	// update cable mesh
	float offset = 250.0;
	FVector start;
	start.X = start.Y = start.Z = 0;
	FVector end = RootComponent->GetComponentToWorld().InverseTransformPosition(Snapped.GetConnectorPos());
	FVector start_t = end;
	end = end + 0.0001;
	if ((FMath::Abs(start_t.X) < 10 || FMath::Abs(start_t.Y) < 10) && FMath::Abs(start_t.Z) <= offset) offset = 1;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;
	Cable->SetStartAndEnd(start, start_t, end, end_t, true);

	// update snap visibilty to
	Adapter->SetVisibility(false, false);
	PoleHolo->SetDisabled(true);
	PlugHolo->SetDisabled(true);
	switch (Snapped.SnapType) {
	case FIN_SETTINGS:
		Adapter->SetVisibility(true, true);
		Adapter->SetRelativeLocation(end);
		Adapter->SetWorldRotation(Snapped.GetConnectorRot());
		break;
	case FIN_POLE:
	case FIN_PLUG:
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetDisabled(false);
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetHologramLocationAndRotation(hit);
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetScrollRotateValue(GetScrollRotateValue());
		break;
	default: ;
	}

	UpdateMeshValidity(validSnap);
}

bool AFINNetworkCableHologram::IsChanged() const {
	return Snapped.SnapType != FIN_NOT_SNAPPED;
}

USceneComponent* AFINNetworkCableHologram::SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) {
	return nullptr;
}

void AFINNetworkCableHologram::SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
	GetRecipes(RecipePole, RecipePlug);
	PoleHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
	PoleHologram1->SetDisabled(true);
	PlugHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
	PlugHologram1->SetDisabled(true);
	PoleHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
	PoleHologram2->SetDisabled(true);
	PlugHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
	PlugHologram2->SetDisabled(true);
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

		OldSnapped = Snapped;
	}
}

void AFINNetworkCableHologram::UpdateMeshValidity(bool bValid) {
	UMaterialInstance* Material = bValid ? mValidPlacementMaterial : mInvalidPlacementMaterial;
	Cable->SetMaterial(0, Material);
	Adapter1->SetMaterial(0, Material);
	Adapter2->SetMaterial(0, Material);
	for (UActorComponent* Comp : PoleHologram1->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	for (UActorComponent* Comp : PoleHologram2->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	for (UActorComponent* Comp : PlugHologram1->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	for (UActorComponent* Comp : PlugHologram2->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
}

bool AFINNetworkCableHologram::IsInSecondStep() {
	return From.SnapType != FIN_NOT_SNAPPED;
}

void AFINNetworkCableHologram::GetRecipes(TSubclassOf<UFGRecipe>& OutRecipePole, TSubclassOf<UFGRecipe>& OutRecipePlug) {
	if (!IsValid(OutRecipePole)) OutRecipePole = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkPole/Recipe_NetworkPole.Recipe_NetworkPole_C"));
	if (!IsValid(OutRecipePlug)) OutRecipePlug = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkWallPlug/Recipe_NetworkWallPlug.Recipe_NetworkWallPlug_C"));
}

void AFINNetworkCableHologram::OnBeginSnap(FFINCablePlacementStepInfo a, bool isValid) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) UFGOutlineComponent::Get(this->GetWorld())->ShowOutline(o, isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED);
	}
}

void AFINNetworkCableHologram::OnEndSnap(FFINCablePlacementStepInfo a) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) UFGOutlineComponent::Get(o->GetWorld())->HideOutline();
	}
}

AFINNetworkCableHologram::AFINNetworkCableHologram() {
	//UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable"));
	UStaticMesh* adapterMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/Mesh_Adapter.Mesh_Adapter"));

	//RecipePole = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkPole/Recipe_NetworkPole.Recipe_NetworkPole_C"));
	//RecipePlug = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkWallPlug/Recipe_NetworkWallPlug.Recipe_NetworkWallPlug_C"));

	this->mMaxPlacementFloorAngle = 90.0f;

	Cable = CreateDefaultSubobject<USplineMeshComponent>(L"Cable");
	Cable->SetMobility(EComponentMobility::Movable);
	Cable->SetupAttachment(RootComponent);
	//Cable->SetStaticMesh(cableMesh);
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
	Adapter1->SetVisibility(false);
	Adapter1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Adapter2 = CreateDefaultSubobject<UStaticMeshComponent>(L"Adapter2");
	Adapter2->SetupAttachment(RootComponent);
	Adapter2->SetMobility(EComponentMobility::Movable);
	Adapter2->SetStaticMesh(adapterMesh);
	Adapter2->SetVisibility(false);
	Adapter2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AFINNetworkCableHologram::~AFINNetworkCableHologram() {}
