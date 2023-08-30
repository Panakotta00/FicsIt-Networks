#include "Components/FINIndicatorPoleHolo.h"
#include "Components/FINIndicatorPole.h"

AFINIndicatorPoleHolo::AFINIndicatorPoleHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINIndicatorPoleHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINIndicatorPoleHolo, Height);
}

void AFINIndicatorPoleHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (LastHeight != Height) {
		for (int i = LastHeight; i > Height; --i) {
			if (i < 0) continue;
			UStaticMeshComponent* Pole = Poles[Poles.Num()-1];
			Poles.Pop();
			if (Pole) {
				Pole->UnregisterComponent();
				Pole->SetActive(false);
				Pole->DestroyComponent();
			}
		}
		UStaticMesh* LongPole = Cast<AFINIndicatorPole>(mBuildClass->GetDefaultObject())->LongPole;
		for (int i = LastHeight; i < Height; ++i) {
			if (i < 0) continue;
			UStaticMeshComponent* Pole = NewObject<UStaticMeshComponent>(this);
			check(Pole);
			Pole->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Pole->SetRelativeLocation(FVector(0,0, -(i) * 100.0));
			Pole->RegisterComponent();
			Pole->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			Pole->SetStaticMesh(LongPole);
			Poles.Add(Pole);
		}
		LastHeight = Height;
	}
}

void AFINIndicatorPoleHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	for (UStaticMeshComponent* Pole : Poles) {
		Pole->UnregisterComponent();
	}
	Poles.Empty();
}

bool AFINIndicatorPoleHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bSnapped) {
		return true;
	} else {
		bSnapped = true;
		SnappedLoc = GetActorLocation();
		
		return false;
	}
}

int32 AFINIndicatorPoleHolo::GetBaseCostMultiplier() const {
	if (bSnapped) return GetHeight(GetActorLocation()) + 6;
	return 0;
}

bool AFINIndicatorPoleHolo::IsValidHitResult(const FHitResult& hitResult) const {
	if (bSnapped) return true;
	return hitResult.bBlockingHit;
}

void AFINIndicatorPoleHolo::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	Super::SetHologramLocationAndRotation(hitResult);
	
	if (bSnapped) {
		float horizontalDistance = FVector::DistXY(hitResult.TraceStart, SnappedLoc);
		float angleOfTrace = FMath::DegreesToRadians((hitResult.TraceEnd - hitResult.TraceStart).Rotation().Pitch);
		float verticalDistance = horizontalDistance * FMath::Tan(angleOfTrace) + hitResult.TraceStart.Z - SnappedLoc.Z;
		Height = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		SetActorLocation(SnappedLoc + FVector(0,0,Height * 100.0));
	} else {
		if (SnappedPole.IsValid()) {
			SetActorLocation(SnappedPole->GetActorLocation() + FVector(0,0,300));
		}
	}
}

void AFINIndicatorPoleHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	AFINIndicatorPole* Pole = Cast<AFINIndicatorPole>(inBuildable);
	Pole->Height = GetHeight(GetActorLocation());
	if (SnappedPole.IsValid()) {
		Pole->BottomConnected = SnappedPole.Get();
		SnappedPole->TopConnected = Pole;
	}
}

void AFINIndicatorPoleHolo::CheckValidFloor() {
	if (!bSnapped) Super::CheckValidFloor();
}

bool AFINIndicatorPoleHolo::TrySnapToActor(const FHitResult& hitResult) {
	if (!bSnapped) {
		AFINIndicatorPole* Pole = Cast<AFINIndicatorPole>(hitResult.GetActor());
		if (Pole && !IsValid(Pole->TopConnected)) {
			SnappedPole = Pole;
			SetActorLocation(Pole->GetActorLocation() + FVector(0,0,300));
			return true;
		}
		SnappedPole = nullptr;
	}
	return false;
}

inline int AFINIndicatorPoleHolo::GetHeight(FVector worldLoc) const {
	return FMath::Clamp(static_cast<int>((worldLoc - SnappedLoc).Z / 100.0), 0, 10);
}
