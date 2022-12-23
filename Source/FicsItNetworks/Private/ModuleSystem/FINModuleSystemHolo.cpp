#include "ModuleSystem/FINModuleSystemHolo.h"

#include "FGColoredInstanceMeshProxy.h"
#include "Buildables/FGBuildable.h"
#include "FGConstructDisqualifier.h"
#include "FicsItNetworksModule.h"
#include "Framework/MultiBox/ToolMenuBase.h"
#include "ModuleSystem/FINModuleSystemModule.h"

AFINModuleSystemHolo::AFINModuleSystemHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

AFINModuleSystemHolo::~AFINModuleSystemHolo() {}

void AFINModuleSystemHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (Snapped && Snapped->GetOwner()->HasAuthority() && bOldIsValid != bIsValid) {
		ForceNetUpdate();
		bOldIsValid = bIsValid;
//		ValidChanged(bIsValid);
	}
}

/*void AFINModuleSystemHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(AFINModuleSystemHolo, bIsValid);
}*/

AActor* AFINModuleSystemHolo::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	FRotator rotation = GetActorRotation();
	FVector location = GetActorLocation();
	FVector scale = GetActorScale3D();

	auto a = Super::Construct(childs, constructionID);
	a->SetActorScale3D(scale);

	Cast<IFINModuleSystemModule>(a)->Execute_setPanel(a, Snapped, (int) SnappedLoc.X, (int) SnappedLoc.Y, (int) SnappedRot);
	
	return a;
}

bool AFINModuleSystemHolo::checkSpace(FVector min, FVector max) {
	if (min.X < 0 || min.X >= Snapped->PanelHeight || min.Y < 0 || min.Y >= Snapped->PanelWidth) return false;
	if (max.X < 0 || max.X >= Snapped->PanelHeight || max.Y < 0 || max.Y >= Snapped->PanelWidth) return false;
	for (int x = (int) min.X; x <= max.X; x++) for (int y = (int) min.Y; y <= max.Y; y++) {
		if (Snapped->GetModule(x, y)) return false;
	}
	return true;
}
FVector AFINModuleSystemHolo::getModuleSize() {
	UObject* module = const_cast<AFGBuildable*>(GetDefault<AFGBuildable>(mBuildClass));
	auto o = Cast<IFINModuleSystemModule>(module);
	int w, h;
	o->Execute_getModuleSize(module, w, h);
	return FVector((float) w, (float) h, 0);
}

bool AFINModuleSystemHolo::IsValidHitResult(const FHitResult& hit) const {
	auto r = GetScrollValue(EHologramScrollMode::HSM_ROTATE);

	USceneComponent* panel = Cast<USceneComponent>(hit.Component.Get());
	while (IsValid(panel) && !panel->IsA<UFINModuleSystemPanel>()) panel = Cast<USceneComponent>(panel->GetAttachParent());

	return IsValid(panel) && panel->IsA<UFINModuleSystemPanel>();
}

bool AFINModuleSystemHolo::TrySnapToActor(const FHitResult& hitResult) {
	USceneComponent* panel_r = Cast<USceneComponent>(hitResult.Component.Get());
	while (IsValid(panel_r) && !panel_r->IsA<UFINModuleSystemPanel>()) panel_r = Cast<USceneComponent>(panel_r->GetAttachParent());
	UFINModuleSystemPanel* panel = Cast<UFINModuleSystemPanel>(panel_r);

	if (!IsValid(panel)) {
		Snapped = nullptr;
		return false;
	}
	Snapped = panel;

	auto loc = Snapped->GetComponentToWorld().InverseTransformPosition(hitResult.Location);
	SnappedLoc = loc;
	SnappedLoc = SnappedLoc / 10.0;
	SnappedLoc.X = floor(SnappedLoc.X);
	SnappedLoc.Y = floor(SnappedLoc.Y);
	SnappedLoc.Z = 0;

	FVector min, max;
	switch (GetScrollRotateValue() % 40) {
	case 0:
		UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot = 0, getModuleSize(), min, max);
		break;
	case -30:
    case 10:
        UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot = 1, getModuleSize(), min, max);
		break;
	case -20:
    case 20:
        UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot = 2, getModuleSize(), min, max);
		break;
	case -10:
    case 30:
        UFINModuleSystemPanel::GetModuleSpace(SnappedLoc, SnappedRot = 3, getModuleSize(), min, max);
		break;
	}
	bIsValid = checkSpace(min, max);
	if (bIsValid) {
		bIsValid = false;
		for (auto& allowed : Snapped->AllowedModules)
			if (mBuildClass->IsChildOf(allowed)) 
				bIsValid = true;
	}
	SetHologramLocationAndRotation(hitResult);
	return true;
}

void AFINModuleSystemHolo::SetHologramLocationAndRotation(const FHitResult& hit) {
	if (!IsValid(Snapped)) return;
	auto loc = SnappedLoc;
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

	if(ShowCompass && IsValid(CompassRose)) {
		//AFGBuildable* module = const_cast<AFGBuildable*>(GetDefault<AFGBuildable>(mBuildClass));
		FVector ActorLocation = { (getModuleSize().X - 1) * 5,  (getModuleSize().Y - 1 ) * 5,0};
		ActorLocation += CompassSurfaceOffset;
		//FVector ArrowLocation = {0, }
		CompassRose->SetRelativeLocation(ActorLocation);
	}
	
	//if(IsValid(CompassComponent)) {
	//	FVector ActorOrigin = {0, 0, 0};
	//	FVector ActorExtent = {0,0,0};
	//	this->GetActorBounds(false, ActorOrigin, ActorExtent);
	//	UE_LOG(LogFicsItNetworks, Display, TEXT("Actor Origin: %s"), *(ActorOrigin.ToString()));
	//	UE_LOG(LogFicsItNetworks, Display, TEXT("Actor Extent: %s"), *(ActorExtent.ToString()));
	//	ActorOrigin-= this->GetActorLocation(); 
	//	ActorOrigin.Z+= FMath::Abs(ActorExtent.Z) + 1;
	//	ActorOrigin.X-= 2;
	//	const FVector ActorLocation = {-2,0,FMath::Abs(ActorExtent.Z) + 1};
	//	CompassComponent->SetRelativeLocation(ActorLocation);
	//	UE_LOG(LogFicsItNetworks, Display, TEXT("New Origin: %s"), *(ActorOrigin.ToString()));
	//	UE_LOG(LogFicsItNetworks, Display, TEXT("New Location: %s"), *(ActorLocation.ToString()));
	//}
}

void AFINModuleSystemHolo::CheckValidPlacement() {
	if (!bIsValid) AddConstructDisqualifier(UFGCDInvalidPlacement::StaticClass());
}

void AFINModuleSystemHolo::BeginPlay() {
	Super::BeginPlay();
	//if(this->GetRecipe()->Getname)

}
void AFINModuleSystemHolo::OnConstruction(const FTransform& MovieSceneBlends) {
	Super::OnConstruction(MovieSceneBlends);

	if(ShowCompass) {
		CompassRose = NewObject<UStaticMeshComponent>(this);
		CompassRose->RegisterComponent();
		CompassRose->SetMobility(EComponentMobility::Movable);
		if(CompassMesh == nullptr || !IsValid(CompassMesh)) {
			UStaticMesh* ArrowMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Components/Helpers/Arrow2.Arrow2"), NULL, LOAD_None, NULL);
			CompassRose->SetStaticMesh(ArrowMesh);
		}else {
			CompassRose->SetStaticMesh(CompassMesh);
		}
		CompassRose->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CompassRose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CompassRose->SetVisibility(true);
		CompassRose->SetRelativeScale3D(CompassScale);
		CompassRose->SetRelativeRotation(CompassRotation);

		FVector ActorLocation = {0,0,0};
		ActorLocation += CompassSurfaceOffset;

		CompassRose->SetRelativeLocation(ActorLocation);
	}
}
