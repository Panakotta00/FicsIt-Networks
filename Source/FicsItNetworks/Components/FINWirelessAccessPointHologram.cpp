#include "FINWirelessAccessPointHologram.h"

#include "FINWirelessAccessPoint.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "FicsItNetworks/FicsItNetworksModule.h"


// Sets default values
AFINWirelessAccessPointHologram::AFINWirelessAccessPointHologram() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	this->mValidHitClasses.Add(AFGBuildableRadarTower::StaticClass());
	this->mScrollMode = EHologramScrollMode::HSM_ROTATE;
}

// Called when the game starts or when spawned
void AFINWirelessAccessPointHologram::BeginPlay() {
	Super::BeginPlay();
}

bool AFINWirelessAccessPointHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	return true;
}

void AFINWirelessAccessPointHologram::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	const auto WirelessAccessPoint = Cast<AFINWirelessAccessPoint>(inBuildable);
	WirelessAccessPoint->AttachedTower = Cast<AFGBuildableRadarTower>(SnappedObj.Get());
}

bool AFINWirelessAccessPointHologram::IsValidHitResult(const FHitResult& hitResult) const {
	return hitResult.Actor.IsValid();
}

bool AFINWirelessAccessPointHologram::TrySnapToActor(const FHitResult& hitResult) {
	// Check if hit actor is valid
	const auto Actor = hitResult.Actor.Get();

	if (Actor && Actor->IsA<AFGBuildableRadarTower>()) {
		IsSnapped = true;
		SnappedObj = Cast<AFGBuildable>(Actor);
		
		const float rotationAngle = GetScrollRotateValue() / 10 * 90;
		const auto rotator = SnappedObj->GetActorRotation().Quaternion();
		const auto actorRotator = rotator * FQuat(FVector::UpVector, FMath::DegreesToRadians(rotationAngle));
		const auto snapRotator = actorRotator * FQuat(FVector::UpVector, PI / 2.0f);
		SetActorRotation(actorRotator);
		SetActorLocation(SnappedObj->GetActorLocation() + snapRotator.RotateVector(FVector(362, 0, 50)));
		
		return true;
	}
	
	IsSnapped = false;
	SnappedObj = nullptr;
	return false;
}

void AFINWirelessAccessPointHologram::CheckValidPlacement() {
	if (!SnappedObj.IsValid() || !SnappedObj.Get()->IsA<AFGBuildableRadarTower>()) {
		AddConstructDisqualifier(UFINCDWirelessAccessPointRequiresTower::StaticClass());
	}
}

void AFINWirelessAccessPointHologram::CheckValidFloor() {}

void AFINWirelessAccessPointHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	Super::SetHologramLocationAndRotation(hitResult);
}

AActor* AFINWirelessAccessPointHologram::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	return Super::Construct(out_children, netConstructionID);
}

