#include "Utils/FINWallAndFoundationHologram.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"

void AFINWallAndFoundationHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	AActor* Actor = hitResult.Actor.Get();
	FVector Location = hitResult.Location;
	FRotator Rotation = FRotator::ZeroRotator;
	if(Actor) {
		if (Actor->IsA<AFGBuildableWall>()) {
			SnapToWall(Cast<AFGBuildableWall>(Actor), hitResult.Normal, hitResult.Location, EAxis::X, FVector(0, 0, 0), 0, Location, Rotation);
		} else if (Actor->IsA<AFGBuildableFoundation>()) {
			bool bTop = hitResult.Normal.Equals(FVector(0, 0, hitResult.Normal.Size()), 0.1);
			bool bBottom = hitResult.Normal.Equals(FVector(0, 0, -hitResult.Normal.Size()), 0.1);
			if (bTop || bBottom) {
				Rotation = Actor->GetActorRotation();
				SnapToFloor(Cast<AFGBuildable>(Actor), Location, Rotation);
				Rotation = (Rotation.Quaternion() + FRotator(bBottom ? -90.0f : 90.0f, 0, 0).Quaternion()).Rotator();
			} else {
				SnapToFoundationSide(Cast<AFGBuildableFoundation>(Actor), Actor->GetActorTransform().InverseTransformRotation(hitResult.Normal.Rotation().Quaternion()).Vector(), EAxis::X, Location, Rotation);
			}
			Location -= Rotation.Vector() * 35;
		}
	}
	SetActorLocationAndRotation(Location, Rotation);
}

bool AFINWallAndFoundationHologram::IsValidHitResult(const FHitResult& hitResult) const {
	AActor* Actor = hitResult.Actor.Get();
	if (!Actor) return false;
	if (Actor->IsA<AFGBuildableWall>() || Actor->IsA<AFGBuildableFoundation>()) return true;
	return false;
}

void AFINWallAndFoundationHologram::CheckValidFloor() {}
