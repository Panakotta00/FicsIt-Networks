#include "FINIndicatorPoleHolo.h"

#include "FINIndicatorPole.h"

AFINIndicatorPoleHolo::AFINIndicatorPoleHolo() {
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
		int Height = Height = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		SetActorLocation(SnappedLoc + FVector(0,0,Height * 100.0));
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
	} else {
		if (SnappedPole.IsValid()) {
			SetActorLocation(SnappedPole->GetActorLocation() + FVector(0,0,300));
		}
	}
}

AActor* AFINIndicatorPoleHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	FRotator rotation = GetActorRotation();
	FVector location = GetActorLocation();
	
	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;

	AFINIndicatorPole* a = GetWorld()->SpawnActor<AFINIndicatorPole>(this->mBuildClass, location, rotation, spawnParams);
	a->SetBuiltWithRecipe(GetRecipe());
	a->Height = GetHeight(GetActorLocation());
	if (SnappedPole.IsValid()) {
		a->BottomConnected = SnappedPole;
		SnappedPole->TopConnected = a;
	}

	// Reset
	bSnapped = false;
	SnappedPole = nullptr;
	for (UStaticMeshComponent* Pole : Poles) {
		Pole->UnregisterComponent();
	}
	Poles.Empty();
	LastHeight = 0;
	
	return UGameplayStatics::FinishSpawningActor(a, FTransform(rotation.Quaternion(), location));
}

void AFINIndicatorPoleHolo::CheckValidFloor() {
	if (!bSnapped) Super::CheckValidFloor();
}

bool AFINIndicatorPoleHolo::TrySnapToActor(const FHitResult& hitResult) {
	if (!bSnapped) {
		AFINIndicatorPole* Pole = Cast<AFINIndicatorPole>(hitResult.Actor.Get());
		if (Pole && !Pole->TopConnected.IsValid()) {
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
