#include "FINWallBoxHolo.h"

#include "FGBuildableBeam.h"
#include "FGBuildablePillar.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"

AFINWallBoxHolo::AFINWallBoxHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINWallBoxHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFINWallBoxHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	SetActorTickEnabled(false);
	
	for (UStaticMeshComponent* Part : Parts) {
		Part->UnregisterComponent();
		Part->SetActive(false);
		Part->DestroyComponent();
	}
	Parts.Empty();
	SetActorHiddenInGame(true);
}

void AFINWallBoxHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AFINWallBoxHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	return true;
}

int32 AFINWallBoxHolo::GetBaseCostMultiplier() const {
	return 1;
}

bool AFINWallBoxHolo::IsValidHitResult(const FHitResult& hitResult) const {
	return hitResult.GetActor() && (hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() || 
	hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>() || 
	hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildablePillar>() || 
	hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableBeam>());
}

//#pragma optimize("", off)
void AFINWallBoxHolo::SetHologramLocationAndRotation(const FHitResult& HitResult) {
	Normal = HitResult.ImpactNormal;
	FVector UpVector = FVector(1,0,0);
	FQuat Quat;
	if (FVector::Coincident(UpVector * -1, Normal) || FVector::Coincident(UpVector, Normal)) {
		Quat = Normal.ToOrientationQuat();
	} else {
		FVector RotationAxis = FVector::CrossProduct(UpVector, Normal);
		RotationAxis.Normalize();
		float DotProduct = FVector::DotProduct(UpVector, Normal);
		float RotationAngle = acosf(DotProduct);
		Quat = FQuat(RotationAxis, RotationAngle);
	}
	const FQuat NewQuat = Quat * FRotator(0, 0, GetScrollRotateValue()).Quaternion();
	const auto Location = HitResult.GetActor()->GetActorLocation();
	const auto Rotation = HitResult.GetActor()->GetActorRotation();
	const auto VectorInActorLocalSpace = Rotation.UnrotateVector(HitResult.ImpactPoint - Location);
	const FVector GridPos = VectorInActorLocalSpace.GridSnap(5);
	const FVector ResDiffPos = GridPos - VectorInActorLocalSpace;
	const FVector ResDiffPosR = Rotation.RotateVector(ResDiffPos);
	const FVector Res = HitResult.ImpactPoint + ResDiffPosR;
	//resDiffPos = FVector::CrossProduct();
	SetActorLocationAndRotation(Res, NewQuat.Rotator());
}
//#pragma optimize("", on)

AActor* AFINWallBoxHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	return Super::Construct(out_children, netConstructionID);
}

void AFINWallBoxHolo::CheckValidFloor() {}

void AFINWallBoxHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);
}
