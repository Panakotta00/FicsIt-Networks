#include "ModuleSystem/FINModuleSystemHolo.h"

#include "Buildables/FGBuildable.h"
#include "FGConstructDisqualifier.h"
#include "FGGameState.h"
#include "ModuleSystem/FINModuleSystemModule.h"
#include "ModuleSystem/FINModuleSystemPanel.h"

AFINModuleSystemHolo::AFINModuleSystemHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	InformationComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InformationDisplay"));
	InformationComponent->SetupAttachment(RootComponent);
	InformationComponent->SetMobility(EComponentMobility::Movable);
	InformationComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AFINModuleSystemHolo::~AFINModuleSystemHolo() {}

void AFINModuleSystemHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (Snapped && Snapped->GetOwner()->HasAuthority() && bOldIsValid != bIsPlacementValid) {
		ForceNetUpdate();
		bOldIsValid = bIsPlacementValid;
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

void AFINModuleSystemHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	if (ApplyDefaultFinishOnConstruction && HasAuthority()) {
		if (IsValid(mBuildClass)) {
			UObject* Object = mBuildClass->GetDefaultObject();
			if (AFGBuildable* Buildable = Cast<AFGBuildable>(Object)) {
				//auto data = Buildable->Execute_GetCustomizationData(Buildable);
				FFactoryCustomizationData CustomizationData = mCustomizationData;
				if (CustomizationData.Data.IsEmpty()) {
					auto v = Buildable->mDefaultSwatchCustomizationOverride;
					CustomizationData.SwatchDesc = Buildable->mDefaultSwatchCustomizationOverride;
					CustomizationData.Initialize(Cast<class AFGGameState>(GetWorld()->GetGameState()));
					inBuildable->SetCustomizationData_Implementation(CustomizationData);
				}
			}
		}
	}
}


bool AFINModuleSystemHolo::IsValidHitResult(const FHitResult& hit) const {
	auto r = GetScrollRotateValue();

	USceneComponent* panel = Cast<USceneComponent>(hit.Component.Get());
	while (IsValid(panel) && !panel->IsA<UFINModuleSystemPanel>()) panel = Cast<USceneComponent>(panel->GetAttachParent());

	return IsValid(panel) && panel->IsA<UFINModuleSystemPanel>();
}

bool AFINModuleSystemHolo::TrySnapToActor(const FHitResult& hitResult) {
	USceneComponent* panel_r = Cast<USceneComponent>(hitResult.Component.Get());
	while (IsValid(panel_r) && !panel_r->IsA<UFINModuleSystemPanel>()) panel_r = Cast<USceneComponent>(panel_r->GetAttachParent());
	UFINModuleSystemPanel* panel = Cast<UFINModuleSystemPanel>(panel_r);
	if(IsValid(Snapped) && Snapped != panel) {
		Snapped->HologramSnapped.Broadcast(false);
	}
	if (!IsValid(panel)) {

		Snapped = nullptr;
		return false;
	}
	UFINModuleSystemPanel* OldSnapped = Snapped;
	Snapped = panel;
	if(Snapped != OldSnapped) {
		panel->HologramSnapped.Broadcast(true);
	}

	FVector loc = Snapped->GetComponentToWorld().InverseTransformPosition(hitResult.Location);
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
	bIsPlacementValid = checkSpace(min, max);
	if (bIsPlacementValid) {
		bIsPlacementValid = false;
		for (auto& allowed : Snapped->AllowedModules) {
			if (mBuildClass->IsChildOf(allowed)) {
				bIsPlacementValid = true;
			}
		}
	}
	SetHologramLocationAndRotation(hitResult);
	return true;
}

void AFINModuleSystemHolo::SetHologramLocationAndRotation(const FHitResult& hit) {
	if (!IsValid(Snapped)) return;
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

	if(ShowCompass && IsValid(CompassRose)) {
		//AFGBuildable* module = const_cast<AFGBuildable*>(GetDefault<AFGBuildable>(mBuildClass));
		FVector ActorLocation = { (getModuleSize().X - 1) * 5,  (getModuleSize().Y - 1 ) * 5,0};
		ActorLocation += CompassSurfaceOffset;
		//FVector ArrowLocation = {0, }
		CompassRose->SetRelativeLocation(ActorLocation);
	}
	if(EnableInformationDisplay && IsValid(InformationComponent)) {
		// Roze is having issues getting the Z position correct for the text.
		const FVector ModuleSize = getModuleSize();
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

void AFINModuleSystemHolo::CheckValidPlacement() {
	if (!bIsPlacementValid) AddConstructDisqualifier(UFGCDInvalidPlacement::StaticClass());
}

void AFINModuleSystemHolo::BeginPlay() {
	Super::BeginPlay();
	//if(this->GetRecipe()->Getname)
	
	UObject* Object = mBuildClass->GetDefaultObject();
	if (AFGBuildable* Buildable = Cast<AFGBuildable>(Object)) {
		if (TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch> DefaultSwatch = Buildable->mDefaultSwatchCustomizationOverride) {
			mCustomizationData.SwatchDesc = DefaultSwatch;
		}
	}

}
void AFINModuleSystemHolo::OnConstruction(const FTransform& MovieSceneBlends) {
	Super::OnConstruction(MovieSceneBlends);

#if UE_GAME
	if(ShowCompass) {
		CompassRose = NewObject<UStaticMeshComponent>(this);
		CompassRose->RegisterComponent();
		CompassRose->SetMobility(EComponentMobility::Movable);
		if(CompassMesh == nullptr || !IsValid(CompassMesh)) {
			UStaticMesh* ArrowMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Buildings/-Shared/Arrow2.Arrow2"), NULL, LOAD_None, NULL);
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
	if(EnableInformationDisplay) {
		InformationComponent->SetVisibility(true);
	}else{
		InformationComponent->SetVisibility(false);
	}
#endif
}

void AFINModuleSystemHolo::Destroyed() {
	Super::Destroyed();
	
	if(IsValid(Snapped)) {
		Snapped->HologramSnapped.Broadcast(false);
	}
}

void AFINModuleSystemHolo::OnInvalidHitResult() {
	Super::OnInvalidHitResult();
	if(IsValid(Snapped)) {
		Snapped->HologramSnapped.Broadcast(false);
		Snapped = nullptr;
	}
}

