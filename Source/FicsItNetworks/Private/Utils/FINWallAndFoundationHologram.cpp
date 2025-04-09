#include "Utils/FINWallAndFoundationHologram.h"

#include "FGBuildableBeam.h"
#include "FGBuildablePillar.h"
#include "FGBuildableWalkway.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"

EFoundationSide AFINWallAndFoundationHologram::GetHitSide(FTransform hitTransform, FVector_NetQuantizeNormal hitNormal)
{
	auto xAxis = hitTransform.GetRotation().GetAxisX().Dot(hitNormal);
	auto yAxis = hitTransform.GetRotation().GetAxisY().Dot(hitNormal);
	auto zAxis = hitTransform.GetRotation().GetAxisZ().Dot(hitNormal);

	if (xAxis >= 0.7071)
		return EFoundationSide::FoundationFront;
	else if (xAxis <= -0.7071)
		return EFoundationSide::FoundationBack;
	else if (yAxis >= 0.7071)
		return EFoundationSide::FoundationRight;
	else if (yAxis <= -0.7071)
		return EFoundationSide::FoundationLeft;
	else if (zAxis >= 0.7071)
		return EFoundationSide::FoundationTop;
	else if (zAxis <= -0.7071)
		return EFoundationSide::FoundationBottom;

	return EFoundationSide::Invalid;
}

void AFINWallAndFoundationHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	AActor* Actor = hitResult.GetActor();
	FVector Location = hitResult.Location;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector UpVector = {0,0,1};
	bool ConcaternateRotation = false;
	if(Actor) {
		if (Actor->IsA<AFGBuildableWall>()) {
			SnapToWall(Cast<AFGBuildableWall>(Actor), hitResult.Normal, hitResult.Location, EAxis::X, FVector(0, 0, 0), 0, Location, Rotation);
		} else if (Actor->IsA<AFGBuildableFoundation>() || Actor->IsA<AFGBuildablePillar>() || Actor->IsA<AFGBuildableBeam>() || Actor->IsA<AFGBuildableWalkway>()) {
			EFoundationSide Side = GetHitSide(Actor->GetTransform(), hitResult.ImpactNormal);
			if(Side == EFoundationSide::FoundationTop) {
				SnapToFloor(static_cast<AFGBuildable*>(Actor), Location, Rotation);
				Rotation+= FRotator(90.0f, 0,0 );
				if(!hitResult.ImpactNormal.Equals(UpVector)) {
					// TODO: Fix Alignment to Normal Vector
				}
				//Rotation = (Rotation.Quaternion() + FRotator(90.0f, 0, 0).Quaternion()).Rotator();	
			}else {
				SnapToFoundationSide(Cast<AFGBuildableFoundation>(Actor), Actor->GetActorTransform().InverseTransformRotation(hitResult.Normal.Rotation().Quaternion()).Vector(), EAxis::X, Location, Rotation);
			}
		}
	}
	AdjustForGround(Location, Rotation);
	Location -= Rotation.Vector() * 35;
	SetActorLocationAndRotation(Location, Rotation);
	this->OnHologramTransformUpdated();
}

bool AFINWallAndFoundationHologram::IsValidHitResult(const FHitResult& hitResult) const {
	AActor* Actor = hitResult.GetActor();
	if (!Actor) return false;
	if (Actor->IsA<AFGBuildableWall>() || Actor->IsA<AFGBuildableFoundation>()) return true;
	return false;
}

void AFINWallAndFoundationHologram::CheckValidFloor() {}
