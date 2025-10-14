// 
#include "Components/FINDynamicModuleSystemHolo.h"

#include "UnrealNetwork.h"

// Sets default values
AFINDynamicModuleSystemHolo::AFINDynamicModuleSystemHolo() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINDynamicModuleSystemHolo::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	ConstructParts();
}

// Called when the game starts or when spawned
void AFINDynamicModuleSystemHolo::BeginPlay() {
	Super::BeginPlay();
	
}

void AFINDynamicModuleSystemHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if ((OldDynamicHeight != DynamicHeight || OldDynamicWidth != DynamicWidth)) {
		OldDynamicHeight = DynamicHeight;
		OldDynamicWidth = DynamicWidth;
		
		ConstructParts();
	}
}

void AFINDynamicModuleSystemHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	SetActorTickEnabled(false);
	
	for (USceneComponent* Part : Parts) {
		Part->UnregisterComponent();
		Part->SetActive(false);
		Part->DestroyComponent();
	}
	Parts.Empty();
	SetActorHiddenInGame(true);
}

void AFINDynamicModuleSystemHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINDynamicModuleSystemHolo, DynamicWidth);
	DOREPLIFETIME(AFINDynamicModuleSystemHolo, DynamicHeight);
}


bool AFINDynamicModuleSystemHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bPlaced) {
		return true;
	} else {
		bPlaced = true;
		return false;
	}
}



AActor* AFINDynamicModuleSystemHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	bPlaced = false;

	return Super::Construct(out_children, netConstructionID);
}

void AFINDynamicModuleSystemHolo::CheckValidFloor() {}


void AFINDynamicModuleSystemHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	if (IFINDynamicModuleBuildable* Buildable = Cast<IFINDynamicModuleBuildable>(inBuildable)) {
		Buildable->Execute_SetDynamicSize(inBuildable, DynamicWidth, DynamicHeight);
	}
}

void AFINDynamicModuleSystemHolo::ConstructParts() {
	// Clear Components
	for (USceneComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();
	
	if (mBuildClass) {
		UObject* Object = mBuildClass->GetDefaultObject();
		auto q = Cast<IFINDynamicModuleBuildable>(Object);
		if (q) {
			q->Execute_SpawnComponents(Object, DynamicWidth, DynamicHeight, this, RootComponent, Parts);
			q->Execute_GetMinimumDynamicSize(Object, MinWidth, MinHeight);
			q->Execute_GetMaximumDynamicSize(Object, MaxWidth, MaxHeight);
			
			RootComponent->SetMobility(EComponentMobility::Movable);
			for (USceneComponent* Part : Parts) {
				Part->SetMobility(EComponentMobility::Movable);
				UMeshComponent* MeshComp = Cast<UMeshComponent>(Part);
				if (MeshComp) {
					MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}
}

void AFINDynamicModuleSystemHolo::OnInvalidHitResult() {
	if (bPlaced) {
		return;
	}
	Super::OnInvalidHitResult();
}

bool AFINDynamicModuleSystemHolo::TrySnapToActor(const FHitResult& hitResult) {
	if (!bPlaced) {
		return Super::TrySnapToActor(hitResult);
	}
	SetHologramLocationAndRotation(hitResult);
	return true;
}

#pragma optimize( "", off )
void AFINDynamicModuleSystemHolo::SetHologramLocationAndRotation(const FHitResult& hit) {
	if (!IsValid(Snapped)) return;
	FVector ModuleSize;
	if (bPlaced) {
		FVector loc = Snapped->GetComponentToWorld().InverseTransformPosition(hit.Location);
		FVector SecondLoc = loc;
		SecondLoc = SecondLoc / 10.0;
		SecondLoc.X = floor(SecondLoc.X);
		SecondLoc.Y = floor(SecondLoc.Y);
		SecondLoc.Z = 0;

		ModuleSize = SecondLoc - SnappedLoc;

		FVector min, max;
		switch (SnappedRot) {
			case 0:
				UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot, ModuleSize, min, max);
				break;
			case 1:
				ModuleSize = FVector(ModuleSize.Y, -ModuleSize.X, 0);
				UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot, ModuleSize, min, max);
				break;
			case 2:
				ModuleSize = FVector(-ModuleSize.X, -ModuleSize.Y, 0);
				UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot, ModuleSize, min, max);
				break;
			case 3:
				ModuleSize = FVector(-ModuleSize.Y, ModuleSize.X, 0);
				UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot, ModuleSize, min, max);
				break;
		}
		bIsPlacementValid = checkSpace(min, max);
		int ModX = ModuleSize.X;
		int ModY = ModuleSize.Y;
		if (ModX != 0) {
			if (!bBidirectionalPlacementAllowed && ModX < 0) {
				ModX = MinWidth;	
			}
			int xf = ModX / abs(ModX);
			DynamicWidth = std::clamp(abs(ModX), MinWidth, MaxWidth) * xf;
		}else {
			DynamicWidth = MinWidth;
		}
		if (ModY != 0) {
			if (!bBidirectionalPlacementAllowed && ModY < 0) {
				ModY = MinHeight;
			}
			int yf = ModY / abs(ModY);
			DynamicHeight = std::clamp(abs(ModY), MinHeight, MaxHeight) * yf;
		}else {
			DynamicHeight = MinHeight;
		}
	}else {
		DynamicWidth = 1;
		DynamicHeight = 1;
		
		FVector loc = SnappedLoc;
		switch (SnappedRot) {
		case 0:
			break;
		case 1:
			loc = loc + FVector{1, 0, 0};
			break;
		case 2:
			loc = loc + FVector{1, 1, 0};
			break;
		case 3:
			loc = loc + FVector{0, 1, 0};
			break;
		}
		loc = loc * 10.0;

		FRotator rot = {0, SnappedRot * 90.0f, 0.0f};
		rot = Snapped->GetComponentToWorld().TransformRotation(rot.Quaternion()).Rotator();
		loc = Snapped->GetComponentToWorld().TransformPosition(loc);
		SetActorScale3D(Snapped->GetComponentScale());
		SetActorLocationAndRotation(loc, rot);
	}

	if(ShowCompass && IsValid(CompassRose)) {
		//AFGBuildable* module = const_cast<AFGBuildable*>(GetDefault<AFGBuildable>(mBuildClass));
		FVector ActorLocation = { (getModuleSize().X - 1) * 5,  (ModuleSize.Y - 1 ) * 5,0};
		ActorLocation += CompassSurfaceOffset;
		//FVector ArrowLocation = {0, }
		CompassRose->SetRelativeLocation(ActorLocation);
	}
	if(EnableInformationDisplay && IsValid(InformationComponent)) {
		// Roze is having issues getting the Z position correct for the text.
		InformationComponent->SetMobility(EComponentMobility::Movable);
		InformationComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FVector ActorLocation = { (ModuleSize.X - 1) * 5,  (ModuleSize.Y - 1 ) * 5,0};
		ActorLocation += InformationDisplayOffset;
		if(ShowCompass && IsValid(CompassRose)) {
			ActorLocation.Z = CompassRose->GetRelativeLocation().Z;   // Dunno why this works, but it does. 
		}
		InformationComponent->SetRelativeLocation(ActorLocation);
		OnInformationUpdate(InformationComponent, hit, Snapped, SnappedLoc, SnappedRot);
	}
}
#pragma optimize( "", on )