#include "FINNetworkCableHologram.h"

#include "FGBlueprintSubsystem.h"
#include "FGConstructDisqualifier.h"
#include "FGOutlineComponent.h"
#include "FINNetworkAdapter.h"
#include "FGPowerConnectionComponent.h"
#include "FINNetworkAdapterHologram.h"
#include "FINNetworkCable.h"
#include "FicsItNetworks/FINComponentUtility.h"
#include "FicsItNetworks/Utils/FINWallAndFoundationHologram.h"

FVector FFINCablePlacementStepInfo::GetConnectorPos() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
		return Cast<USceneComponent>(SnappedObj)->GetComponentLocation();
	case FIN_ADAPTER: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		return Holo->GetActorLocation();
	} case FIN_POLE: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		AFGBuildable* Pole = AFINNetworkCableHologram::GetDefaultBuildable_Static<AFGBuildable>(Holo);
		return Cast<AActor>(SnappedObj)->GetActorLocation() + FVector(0,0,635);
	} case FIN_PLUG: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		TArray<UFINNetworkConnectionComponent*> Connector = UFINComponentUtility::GetComponentsFromSubclass<UFINNetworkConnectionComponent>(AFINNetworkCableHologram::GetBuildClass(Holo));
		if (Connector.Num() > 0) return Holo->GetTransform().TransformPosition(Connector[0]->GetRelativeLocation());
		else return Holo->GetActorLocation();
	} default:
		return FVector::ZeroVector;
	}
}

FRotator FFINCablePlacementStepInfo::GetConnectorRot() const {
	switch (SnapType) {
	case FIN_CONNECTOR: {
		USceneComponent* Component = Cast<USceneComponent>(SnappedObj);
		return Component->GetComponentRotation();
	}
	case FIN_ADAPTER:
	case FIN_POLE:
	case FIN_PLUG: {
		AFGBuildableHologram* Holo = Cast<AFGBuildableHologram>(SnappedObj);
		check(Holo);
		return Holo->GetActorRotation();
	} default:
		return FRotator::ZeroRotator;
	}
}

AActor* FFINCablePlacementStepInfo::GetActor() const {
	switch (SnapType) {
	case FIN_CONNECTOR:
		return Cast<UFINNetworkConnectionComponent>(SnappedObj)->GetAttachmentRootActor();
	case FIN_ADAPTER: {
		AFINNetworkAdapterHologram* Holo = Cast<AFINNetworkAdapterHologram>(SnappedObj);
		if (Holo && Holo->bSnapped) {
			return Holo->PrevSnappedActor;
		} else {
			return nullptr;
		}
	} default:
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
}

void AFINNetworkCableHologram::BeginPlay() {
	Super::BeginPlay();

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
	case FIN_ADAPTER: {
		return nullptr;
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
	if (From.SnapType == FIN_POLE || From.SnapType == FIN_PLUG || From.SnapType == FIN_ADAPTER) {
		AActor* Child = childs[i--];
		UFINNetworkConnectionComponent* Connector = Cast<UFINNetworkConnectionComponent>(Child->GetComponentByClass(UFINNetworkConnectionComponent::StaticClass()));
		Connector1Cache = Connector;
	}
	if (Snapped.SnapType == FIN_POLE || Snapped.SnapType == FIN_PLUG || Snapped.SnapType == FIN_ADAPTER) {
		AActor* Child = childs[i--];
		UFINNetworkConnectionComponent* Connector = Cast<UFINNetworkConnectionComponent>(Child->GetComponentByClass(UFINNetworkConnectionComponent::StaticClass()));
		Connector2Cache = Connector;
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
	AFGBuildableHologram* AdapterHolo, *PlugHolo, *PoleHolo;
	GetHolos(AdapterHolo, PlugHolo, PoleHolo);
	
	if (AdapterHolo) AdapterHolo->SetDisabled(true);
	if (PoleHolo) PoleHolo->SetDisabled(true);
	if (PlugHolo) PlugHolo->SetDisabled(true);

	bool bAdapterSnapped = false;
	if (bEnableAdapter && AdapterHolo) bAdapterSnapped = AdapterHolo->TrySnapToActor(hitResult);
	
	// Check if hit actor is valid
	AActor* actor = hitResult.Actor.Get();
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
		UpdateSnapping(hitResult);
		return true;
	}

	// Try snap to new adapter
	if (bEnableAdapter && AdapterHolo && bAdapterSnapped) {
		Snapped = {FIN_ADAPTER, AdapterHolo};
		UpdateSnapping(hitResult);
		return true;
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
	if (AFINNetworkAdapterHologram* FromAdapterHolo = Cast<AFINNetworkAdapterHologram>(From.SnappedObj)) {
		if (AFINNetworkAdapterHologram* ToAdapterHolo = Cast<AFINNetworkAdapterHologram>(Snapped.SnappedObj)) {
			if (FromAdapterHolo->bSnapped && FromAdapterHolo->PrevSnappedActor == ToAdapterHolo->PrevSnappedActor) {
				AddConstructDisqualifier(UFGCDWireSnap::StaticClass());
				ret = false;
			}
		}
	}
	if (Snapped.SnapType == FIN_CONNECTOR) {
		UFINNetworkConnectionComponent* Connector = Cast<UFINNetworkConnectionComponent>(Snapped.SnappedObj);
		if (Connector->ConnectedCables.Num() >= Connector->MaxCables) {
			AddConstructDisqualifier(UFGCDWireTooManyConnections::StaticClass());
			ret = false;
		}
		if (!Connector->GetAllowedCableConnections().Contains(GetItemDescriptor())) {
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
	if (From.SnapType != FIN_NOT_SNAPPED && Snapped.SnapType != FIN_NOT_SNAPPED) {
		if ((From.GetConnectorPos() - Snapped.GetConnectorPos()).Size() > 10000.0f) {
			AddConstructDisqualifier(UFGCDWireTooLong::StaticClass());
			ret = false;
		}
		AFGBlueprintSubsystem* BPSubSys = AFGBlueprintSubsystem::GetBlueprintSubsystem(this);
		bool bFromInDesigner = false, bToInDesigner = false;
		if (AFGBuildableHologram* FromHolo = Cast<AFGBuildableHologram>(From.SnappedObj)) {
			bFromInDesigner = !!FromHolo->GetBlueprintDesigner();
		} else if (USceneComponent* FromScene = Cast<USceneComponent>(From.SnappedObj)) {
			AFGBuildable* Buildable = Cast<AFGBuildable>(FromScene->GetOwner());
			if (Buildable) bFromInDesigner = !!Buildable->GetBlueprintDesigner();
			else bFromInDesigner = !!BPSubSys->IsLocationInsideABlueprintDesigner(FromScene->GetComponentLocation());
		}
		if (AFGBuildableHologram* ToHolo = Cast<AFGBuildableHologram>(Snapped.SnappedObj)) {
			bToInDesigner = !!ToHolo->GetBlueprintDesigner();
		} else if (USceneComponent* ToScene = Cast<USceneComponent>(Snapped.SnappedObj)) {
			AFGBuildable* Buildable = Cast<AFGBuildable>(ToScene->GetOwner());
			if (Buildable) bToInDesigner = !!Buildable->GetBlueprintDesigner();
			else bToInDesigner = !!BPSubSys->IsLocationInsideABlueprintDesigner(ToScene->GetComponentLocation());
		}
		if (bFromInDesigner != bToInDesigner) {
			AddConstructDisqualifier(UFGDesignerWorldCommingling::StaticClass());
		}
	}
	return ret;
}

void AFINNetworkCableHologram::SetHologramLocationAndRotation(const FHitResult& hit) {
	AFGBuildableHologram *AdapterHolo, *PlugHolo, *PoleHolo;
	GetHolos(AdapterHolo, PlugHolo, PoleHolo);
	
	AActor* Actor = hit.GetActor();
	if (Actor) {
		if (bEnablePlug && PlugHolo && (Actor->IsA<AFGBuildableFoundation>() || Actor->IsA<AFGBuildableWall>()) && !hit.Normal.Equals(FVector(0, 0, 1), 0.1)) {
			Snapped = { FIN_PLUG, PlugHolo };
			PlugHolo->SetHologramLocationAndRotation(hit);
		} else if (bEnablePole && PoleHolo) {
			Snapped = { FIN_POLE, PoleHolo };
			PoleHolo->SetHologramLocationAndRotation(hit);
		} else {
			Snapped = {FIN_NOT_SNAPPED, nullptr};
		}
	}

	UpdateSnapping(hit);
}

void AFINNetworkCableHologram::UpdateCable() {
	float MaxSlack = 250;
	float DistanceFactor = 1;
	AFINNetworkCable* LocalCable = Cast<AFINNetworkCable>(mBuildClass->GetDefaultObject());
	if(LocalCable != nullptr) {
		MaxSlack = LocalCable->MaxCableSlack;
		DistanceFactor = LocalCable->SlackLengthFactor;
	}
	// update cable mesh
	float Offset = 250.0;
	FVector Start;
	Start.X = Start.Y = Start.Z = 0;
	FVector End = RootComponent->GetComponentToWorld().InverseTransformPosition(Snapped.GetConnectorPos());
	FVector Start_T = End;
	End = End + 0.0001;
	if ((FMath::Abs(Start_T.X) < 10 || FMath::Abs(Start_T.Y) < 10) && FMath::Abs(Start_T.Z) <= Offset) Offset = 1;
	const int Length = FVector::Distance(Start, End);
	Offset = FMath::Min(Length * DistanceFactor , MaxSlack);
	Start_T.Z -= Offset;
	FVector End_T = End;
	End_T.Z += Offset;
	Cable->SetStartAndEnd(Start, Start_T, End, End_T, true);
}

void AFINNetworkCableHologram::UpdateChildHolos(const FHitResult& HitResult) {
	AFGBuildableHologram *AdapterHolo, *PlugHolo, *PoleHolo;
	GetHolos(AdapterHolo, PlugHolo, PoleHolo);
	
	if (AdapterHolo) AdapterHolo->SetDisabled(true);
	if (PoleHolo) PoleHolo->SetDisabled(true);
	if (PlugHolo) PlugHolo->SetDisabled(true);

	switch (Snapped.SnapType) {
	case FIN_POLE:
	case FIN_PLUG:
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetScrollRotateValue(GetScrollRotateValue());
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetHologramLocationAndRotation(HitResult);
	case FIN_ADAPTER:
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetDisabled(false);
		break;
	default: ;
	}
}

void AFINNetworkCableHologram::UpdateSnapping(const FHitResult& HitResult) {
	// place to snap start
	if (!IsInSecondStep()) this->RootComponent->SetWorldLocation(Snapped.GetConnectorPos());
	else this->RootComponent->SetWorldLocation(From.GetConnectorPos());

	// update snapp data and sub holos
	UpdateSnappingEffects();
	
	bool validSnap = IsSnappedValid();
	UpdateMeshValidity(validSnap);
	UpdateCable();
	UpdateChildHolos(HitResult);
	UpdateMeshValidity(validSnap);
}

bool AFINNetworkCableHologram::IsChanged() const {
	return Snapped.SnapType != FIN_NOT_SNAPPED;
}

USceneComponent* AFINNetworkCableHologram::SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) {
	return nullptr;
}

void AFINNetworkCableHologram::SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
	if (RecipePole) {
		PoleHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
		PoleHologram1->SetDisabled(true);
	}
	if (RecipePlug) {
		PlugHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
		PlugHologram1->SetDisabled(true);
	}
	if (RecipeAdapter) {
		AdapterHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipeAdapter, hologramOwner, spawnLocation, hologramInstigator));
		AdapterHologram1->SetDisabled(true);
	}
	// First The Holograms for the first step, then for the next step. Order needed for later child construction and ordering in construction.
	if (RecipePole) {
		PoleHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
		PoleHologram2->SetDisabled(true);
	}
	if (RecipePlug) {
		PlugHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
		PlugHologram2->SetDisabled(true);
	}
	if (RecipeAdapter) {
		AdapterHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipeAdapter, hologramOwner, spawnLocation, hologramInstigator));
		AdapterHologram2->SetDisabled(true);
	}
}

void AFINNetworkCableHologram::OnInvalidHitResult() {
	OnEndSnap(Snapped);
	OnEndSnap(OldSnapped);
	Snapped.SnapType = OldSnapped.SnapType = FIN_NOT_SNAPPED;
}

void AFINNetworkCableHologram::UpdateSnappingEffects() {
	if (Snapped.SnappedObj != OldSnapped.SnappedObj || Snapped.SnapType != OldSnapped.SnapType) {
		OnEndSnap(OldSnapped);
		OnBeginSnap(Snapped, IsSnappedValid());

		OldSnapped = Snapped;
	}
}

void AFINNetworkCableHologram::UpdateMeshValidity(bool bValid) {
	UMaterialInstance* Material = bValid ? mValidPlacementMaterial : mInvalidPlacementMaterial;
	SetMaterial(Material);
	Cable->SetMaterial(0, Material);
	if (Snapped.SnapType == FIN_NOT_SNAPPED || (IsInSecondStep() && From.SnapType == FIN_NOT_SNAPPED)) {
		Cable->SetVisibility(false, true);
	} else {
		Cable->SetVisibility(true, true);
	}
}

bool AFINNetworkCableHologram::IsInSecondStep() {
	return From.SnapType != FIN_NOT_SNAPPED;
}

void AFINNetworkCableHologram::OnBeginSnap(FFINCablePlacementStepInfo a, bool isValid) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) {
			UFGOutlineComponent* Outline = UFGOutlineComponent::Get(this->GetWorld());
			if (Outline) Outline->ShowOutline(o, isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED);
		}
	}
}

void AFINNetworkCableHologram::OnEndSnap(FFINCablePlacementStepInfo a) {
	if (a.SnapType != FIN_NOT_SNAPPED) {
		AActor* o = a.GetActor();
		if (o) {
			UFGOutlineComponent* Outline = UFGOutlineComponent::Get(this->GetWorld());
			Outline->HideOutline();
		}
	}
}

AFINNetworkCableHologram::AFINNetworkCableHologram() {
	//UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable"));
	
	//RecipePole = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkPole/Recipe_NetworkPole.Recipe_NetworkPole_C"));
	//RecipePlug = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkWallPlug/Recipe_NetworkWallPlug.Recipe_NetworkWallPlug_C"));

	this->mNeedsValidFloor = false;
	SetSnapToGuideLines(false);

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
}

AFINNetworkCableHologram::~AFINNetworkCableHologram() {}

void AFINNetworkCableHologram::GetHolos(AFGBuildableHologram*& OutAdapterHolo, AFGBuildableHologram*& OutPlugHolo, AFGBuildableHologram*& OutPoleHolo) {
	OutAdapterHolo = IsInSecondStep() ? AdapterHologram1 : AdapterHologram2;
	OutPlugHolo = IsInSecondStep() ? PlugHologram1 : PlugHologram2;
	OutPoleHolo = IsInSecondStep() ? PoleHologram1 : PoleHologram2;
}
