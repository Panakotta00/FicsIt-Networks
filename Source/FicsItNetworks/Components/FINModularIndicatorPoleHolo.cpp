#include "FINModularIndicatorPoleHolo.h"
#include "FINModularIndicatorPole.h"


DEFINE_LOG_CATEGORY(LogFicsItNetworks_DebugRoze);

AFINModularIndicatorPoleHolo::AFINModularIndicatorPoleHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINModularIndicatorPoleHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINModularIndicatorPoleHolo, Extension);
	DOREPLIFETIME(AFINModularIndicatorPoleHolo, Vertical);
}

void AFINModularIndicatorPoleHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(Vertical != LastVertical || Extension != LastExtension) {
		LastExtension = Extension;
		LastVertical = Vertical;
		
		RerunConstructionScripts();
	}
	
}


void AFINModularIndicatorPoleHolo::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	// Clear Components
	for (UStaticMeshComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();

	// Create Components
	if (mBuildClass) {
		UStaticMesh* AM;
		UStaticMesh* BM;
		UStaticMesh* EM;
		UStaticMesh* CM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->ConnectorMesh;
		if(Vertical) {
			BM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->VerticalBaseMesh;
			EM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->VerticalExtensionMesh;
			AM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->VerticalAttachmentMesh;
		}else{
			BM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->NormalBaseMesh;
			EM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->NormalExtensionMesh;
			AM = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject())->NormalAttachmentMesh;
		}
		AFINModularIndicatorPole::SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, BM, EM, AM, CM, this, RootComponent, Parts);
		RootComponent->SetMobility(EComponentMobility::Movable);
		for (UStaticMeshComponent* Part : Parts) {
			Part->SetMobility(EComponentMobility::Movable);
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AFINModularIndicatorPoleHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	SetActorTickEnabled(false);
	for (UStaticMeshComponent* Part : Parts) {
		Part->UnregisterComponent();
	}
	Parts.Empty();
	SetActorTickEnabled(true);
}

bool AFINModularIndicatorPoleHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bSnapped) {
		return true;
	} else {
		bSnapped = true;
		SnappedLoc = GetActorLocation();
		
		return false;
	}
}

int32 AFINModularIndicatorPoleHolo::GetBaseCostMultiplier() const {
	return 1;
}

bool AFINModularIndicatorPoleHolo::IsValidHitResult(const FHitResult& HitResult) const {
	if (bSnapped) return true;
	return HitResult.GetActor() && (HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() || HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>());
}

void AFINModularIndicatorPoleHolo::SetHologramLocationAndRotation(const FHitResult& HitResult) {
	if (bSnapped) {
		float horizontalDistance = FVector::DistXY(HitResult.TraceStart, SnappedLoc);
		float angleOfTrace = FMath::DegreesToRadians((HitResult.TraceEnd - HitResult.TraceStart).Rotation().Pitch);
		if(Vertical) {
			float verticalDistance = horizontalDistance * FMath::Tan(angleOfTrace) + HitResult.TraceStart.Z - SnappedLoc.Z;
			Extension = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		}else {
			float verticalDistance = horizontalDistance * FMath::Tan(angleOfTrace) + HitResult.TraceStart.Z - SnappedLoc.Z;
			Extension = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		}
	} else {
		if(HitResult.GetActor()) {
			if(HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>()) {
				Vertical = true;
			}else {
				Vertical = false;
			}
		}
		Normal = HitResult.ImpactNormal;
		FVector UpVector;
		if(Vertical) {
			UpVector = FVector(1,0,0);
		}else {
			UpVector = FVector(0, 0, 1);
		}
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
		FQuat NewQuat;
		if(Vertical) {
			NewQuat = Quat * FRotator(0, 0, GetScrollRotateValue()).Quaternion();
		}else {
			NewQuat = HitResult.GetActor()->GetActorRotation().Quaternion() * FRotator(0,0,0).Quaternion() * FRotator(0, GetScrollRotateValue(), 0).Quaternion();
		}
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
}

void AFINModularIndicatorPoleHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	AFINModularIndicatorPole* Pole = Cast<AFINModularIndicatorPole>(inBuildable);
	Pole->Extension = Extension;
	Pole->Vertical = Vertical;
}

void AFINModularIndicatorPoleHolo::CheckValidFloor() {
	if (!bSnapped) Super::CheckValidFloor();
}

bool AFINModularIndicatorPoleHolo::TrySnapToActor(const FHitResult& hitResult) {
	return false;
}

inline int AFINModularIndicatorPoleHolo::GetHeight(FVector worldLoc) const {
	if(Vertical) {
		return FMath::Clamp(static_cast<int>((worldLoc - SnappedLoc).Z / 10.6), 1, 10);
	}else {
		return FMath::Clamp(static_cast<int>((worldLoc - SnappedLoc).Z / 26.6992), 1, 10);
	}
}
