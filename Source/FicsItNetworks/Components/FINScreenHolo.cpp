#include "FINScreenHolo.h"

#include "FGBuildableFoundation.h"
#include "FGBuildableWall.h"
#include "FINScreen.h"

AFINScreenHolo::AFINScreenHolo() {}

bool AFINScreenHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bSnapped) {
		return true;
	} else {
		bSnapped = true;
		return false;
	}
}

int32 AFINScreenHolo::GetBaseCostMultiplier() const {
	return FMath::Abs(ScreenWidth) * FMath::Abs(ScreenHeight);
}

bool AFINScreenHolo::IsValidHitResult(const FHitResult& hitResult) const {
	if (bSnapped) return true;
	return hitResult.GetActor() && (hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() || hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>());
}

void AFINScreenHolo::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	if (bSnapped) {
		// Calculate new Screen Size
		FVector PlaneNormal = Normal;
		PlaneNormal.Normalize(0.01);
		FVector PlanePosition = GetActorLocation();
		FVector LineDirection = (hitResult.TraceEnd - hitResult.TraceStart);
		LineDirection.Normalize(0.01);
		FVector LinePosition = hitResult.TraceStart;
		float PlaneDotLine = FVector::DotProduct(PlaneNormal, LineDirection);
		if (PlaneDotLine == 0.0) {
			ScreenHeight = 1;
			ScreenWidth = 1;
		} else {
			double t = (FVector::DotProduct(PlaneNormal, PlanePosition) - FVector::DotProduct(PlaneNormal, LinePosition)) / PlaneDotLine;
			FVector ViewPostion = LinePosition + (LineDirection * t);
			FVector ScreenDiagonal = ViewPostion - PlanePosition;
			FVector TestDiagonal = GetActorRotation().UnrotateVector(ScreenDiagonal);
			ScreenHeight = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Z + (TestDiagonal.Z < 0 ? -100.0 : 100.0)) / 100.0), -10, 10);
			ScreenWidth = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Y + (TestDiagonal.Y < 0 ? -100.0 : 100.0)) / 100.0), -10, 10);
			if (ScreenHeight == 0) ScreenHeight = 1;
			if (ScreenWidth == 0) ScreenWidth = 1;
		}
	} else {
		Normal = hitResult.ImpactNormal;
		ScreenHeight = 1;
		ScreenWidth = 1;
		SetActorLocationAndRotation(hitResult.ImpactPoint, Normal.Rotation() + FRotator(0,0, GetScrollRotateValue()));
	}

	if (OldScreenHeight != ScreenHeight || OldScreenWidth != ScreenWidth) {
		OldScreenHeight = ScreenHeight;
		OldScreenWidth = ScreenWidth;
		
		// Clear Components
		for (UStaticMeshComponent* comp : Parts) {
			comp->UnregisterComponent();
			comp->SetActive(false);
			comp->DestroyComponent();
		}
		Parts.Empty();

		// Create Components
		UStaticMesh* MiddlePartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenMiddle;
		UStaticMesh* EdgePartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenEdge;
		UStaticMesh* CornerPartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenCorner;
		AFINScreen::SpawnComponents(ScreenWidth, ScreenHeight, MiddlePartMesh, EdgePartMesh, CornerPartMesh, this, RootComponent, Parts);
		for (UStaticMeshComponent* Part : Parts) {
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

AActor* AFINScreenHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	bSnapped = false;

	FRotator rotation = GetActorRotation();
	FVector location = GetActorLocation();
	
	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;

	AFINScreen* a = GetWorld()->SpawnActor<AFINScreen>(this->mBuildClass, location, rotation, spawnParams);
	a->SetBuiltWithRecipe(GetRecipe());
	a->ScreenHeight = ScreenHeight;
	a->ScreenWidth = ScreenWidth;
	
	// Clear Components
	for (UStaticMeshComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();
	
	return UGameplayStatics::FinishSpawningActor(a, FTransform(rotation.Quaternion(), location));
}

void AFINScreenHolo::CheckValidFloor() {}
