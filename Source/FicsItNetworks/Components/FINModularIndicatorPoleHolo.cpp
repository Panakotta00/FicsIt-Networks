#include "FINModularIndicatorPoleHolo.h"

#include <algorithm>

#include "FGBuildableBeam.h"
#include "FGBuildablePillar.h"
#include "FINModularIndicatorPole.h"
#include "Buildables/FGBuildableFactoryBuilding.h"
#include "Kismet/KismetMathLibrary.h"


DEFINE_LOG_CATEGORY(LogFicsItNetworks_DebugRoze);


UBuildMode_Auto::UBuildMode_Auto() {
	mDisplayName = FText::FromString("Auto");
}

UBuildMode_OnVertical::UBuildMode_OnVertical() {
	mDisplayName = FText::FromString("Vertical Mode");
}

UBuildMode_OnHorizontal::UBuildMode_OnHorizontal() {
	mDisplayName = FText::FromString("Horizontal Mode");
}

AFINModularIndicatorPoleHolo::AFINModularIndicatorPoleHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	mBuildModeAuto = UBuildMode_Auto::StaticClass();
	mDefaultBuildMode = mBuildModeAuto;
	mBuildModeOnHorizontalSurface = UBuildMode_OnHorizontal::StaticClass();
	mBuildModeOnVerticalSurface = UBuildMode_OnVertical::StaticClass();
}

void AFINModularIndicatorPoleHolo::GetSupportedBuildModes_Implementation(
	TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const {
	Super::GetSupportedBuildModes_Implementation(out_buildmodes);
	out_buildmodes.Add(mBuildModeAuto);
	out_buildmodes.Add(mBuildModeOnHorizontalSurface);
	out_buildmodes.Add(mBuildModeOnVerticalSurface);
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
		const AFINModularIndicatorPole* Pole = Cast<AFINModularIndicatorPole>(mBuildClass->GetDefaultObject());
		if(Vertical) {
			AFINModularIndicatorPole::SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, Pole->VerticalBaseMesh, Pole->VerticalExtensionMesh, Pole->VerticalAttachmentMesh, Pole->ConnectorMesh, this, RootComponent, Parts,
													  Pole->VerticalBaseOffset, Pole->VerticalExtensionOffset, Pole->VerticalExtensionMultiplier, Pole->VerticalAttachmentOffset,
													  Pole->VerticalConnectorMeshOffset, Pole->VerticalConnectorMeshRotation, Pole->VerticalConnectorMeshScale);
		}else{
			AFINModularIndicatorPole::SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, Pole->NormalBaseMesh, Pole->NormalExtensionMesh, Pole->NormalAttachmentMesh, Pole->ConnectorMesh, this, RootComponent, Parts,
													  Pole->HorizontalBaseOffset, Pole->HorizontalExtensionOffset, Pole->HorizontalExtensionMultiplier, Pole->HorizontalAttachmentOffset,
													  Pole->HorizontalConnectorMeshOffset, Pole->HorizontalConnectorMeshRotation, Pole->HorizontalConnectorMeshScale);
		}
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
		SetScrollRotateValue(0);
		return false;
	}
}

int32 AFINModularIndicatorPoleHolo::GetBaseCostMultiplier() const {
	return 1;
}

bool AFINModularIndicatorPoleHolo::IsValidHitResult(const FHitResult& HitResult) const {
	if (bSnapped) return true;
	return true;
	//return HitResult.GetActor() && (HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() ||
	//	HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>() || 
	//	HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildablePillar>() || 
	//	HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableBeam>() 
	//);
}

int AFINModularIndicatorPoleHolo::GetHitSideSingleAxis(const FVector A, const FVector B) {
	float Q = FVector::DotProduct(A, B);
	if(Q >= 0.7071) {
		return 1;		
	}
	if(Q <= -0.7071) {
		return -1;
	}
	return 0;
}
//#pragma optimize("", off)
EFoundationSide AFINModularIndicatorPoleHolo::GetHitSide(FVector AxisX, FVector AxisY, FVector AxisZ, FVector HitNormal) {
	int Q = GetHitSideSingleAxis(AxisX, HitNormal);
	if(Q > 0) {
		return EFoundationSide::FoundationFront;
	}
	if(Q < 0){
		return EFoundationSide::FoundationBack;
	}
	Q = GetHitSideSingleAxis(AxisY, HitNormal);
	if(Q > 0) {
		return EFoundationSide::FoundationRight;
	}
	if(Q < 0){
		return EFoundationSide::FoundationLeft;
	}
	Q = GetHitSideSingleAxis(AxisZ, HitNormal);
	if(Q > 0) {
		return EFoundationSide::FoundationTop;
	}
	if(Q < 0){
		return EFoundationSide::FoundationBottom;
	}
	return EFoundationSide::Invalid;
}
//#pragma optimize("", on)

void AFINModularIndicatorPoleHolo::SetHologramLocationAndRotation(const FHitResult& HitResult) {
	if (bSnapped) {
		float horizontalDistance = FVector::DistXY(HitResult.TraceStart, SnappedLoc);
		float angleOfTrace = FMath::DegreesToRadians((HitResult.TraceEnd - HitResult.TraceStart).Rotation().Pitch);
		Extension = FMath::Clamp(GetScrollRotateValue() / 10 + 1, 1, 10);
;;;;;		//if(Vertical) {
		//	float verticalDistance = horizontalDistance * FMath::Tan(angleOfTrace) + HitResult.TraceStart.Z - SnappedLoc.Z;
		//	Extension = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		//}else {
		//	float verticalDistance = horizontalDistance * FMath::Tan(angleOfTrace) + HitResult.TraceStart.Z - SnappedLoc.Z;
		//	if(UpsideDown) {
		//		Extension = GetHeight(SnappedLoc + FVector(0,0,-verticalDistance));
		//	}else{
		//		Extension = GetHeight(SnappedLoc + FVector(0,0,verticalDistance));
		//	}
		//}
	} else {
		UpsideDown = false;
		if(mCurrentBuildMode == mBuildModeAuto) {
			if(HitResult.GetActor()) {
				if(HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>()) {
					Vertical = true;
				}else if(HitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableBeam>()){
					FVector VX, VY, VZ;
					UKismetMathLibrary::GetAxes(HitResult.GetActor()->GetActorRotation(), VX, VY, VZ);
					//const auto q = FFoundationHelpers::FindBestMatchingFoundationSideFromLocalNormal(Normal);
					const auto q = GetHitSide(VX, VY, VZ, HitResult.Normal);
					switch(q) {
					case EFoundationSide::FoundationBottom:{
						Vertical = false;
						UpsideDown = true;
						break;
					}
					case EFoundationSide::FoundationTop: {
						Vertical = false;
						break;
					}
					case EFoundationSide::FoundationBack: case EFoundationSide::FoundationFront: case EFoundationSide::FoundationLeft: case EFoundationSide::FoundationRight: {
						Vertical = true;
						break;
					}
					default: Vertical = true;
					}	
				}else {
					FVector VX, VY, VZ;
					UKismetMathLibrary::GetAxes(HitResult.GetActor()->GetActorRotation(), VX, VY, VZ);
					//const auto q = FFoundationHelpers::FindBestMatchingFoundationSideFromLocalNormal(Normal);
					const auto q = GetHitSide(VX, VY, VZ, HitResult.Normal);
					switch(q) {
					case EFoundationSide::FoundationBottom:{
						Vertical = false;
						UpsideDown = true;
						break;
					}
					case EFoundationSide::FoundationTop: {
						Vertical = false;
						break;
					}
					case EFoundationSide::FoundationBack: case EFoundationSide::FoundationFront: case EFoundationSide::FoundationLeft: case EFoundationSide::FoundationRight: {
						Vertical = true;
						break;
					}
					default: Vertical = true;
					}	
				}
			}
		}else if(mCurrentBuildMode == mBuildModeOnHorizontalSurface) {
			Vertical = false;
			FVector VX, VY, VZ;
			UKismetMathLibrary::GetAxes(HitResult.GetActor()->GetActorRotation(), VX, VY, VZ);
			//const auto q = FFoundationHelpers::FindBestMatchingFoundationSideFromLocalNormal(Normal);
			const auto q = GetHitSide(VX, VY, VZ, HitResult.Normal);
			if(q == EFoundationSide::FoundationBottom) {
				UpsideDown = true;
			}
		}else if(mCurrentBuildMode == mBuildModeOnVerticalSurface) {
			Vertical = true;
		}
		
		Normal = HitResult.ImpactNormal;
		FVector UpVector;
		if(Vertical) {
			UpVector = FVector(1,0,0);
		}else {
			UpVector = FVector(0, 0, 1);
		}
		FQuat Quat;
		if (FVector::Coincident(UpVector, Normal)) {
			Quat = Normal.ToOrientationQuat();
		}else if(FVector::Coincident(UpVector * -1, Normal)) {
			Quat = Normal.ToOrientationQuat() * -1;
		} else {
			if(mCurrentBuildMode == mBuildModeAuto) {
				Quat = Normal.ToOrientationQuat();
			}else{
				FVector RotationAxis = FVector::CrossProduct(UpVector, Normal);
				RotationAxis.Normalize();
				float DotProduct = FVector::DotProduct(UpVector, Normal);
				float RotationAngle = acosf(DotProduct);
				Quat = FQuat(RotationAxis, RotationAngle);
			}
		}
		FQuat NewQuat;
		if(Vertical) {
			NewQuat = Quat * FRotator(0, 0, GetScrollRotateValue()).Quaternion();
		}else {
			if(mCurrentBuildMode == mBuildModeOnHorizontalSurface) {
				//NewQuat = Normal.ToOrientationQuat();
				//NewQuat = UKismetMathLibrary::MakeRotationFromAxes(Normal, FVector(0,1, 0), FVector(0,0,1)).Quaternion();
				//NewQuat = UKismetMathLibrary::MakeRotFromX(Normal).Quaternion();
				//NewQuat*= FRotator(0,90,0).Quaternion();
				//if(UpsideDown){
				//	NewQuat*= FRotator(0,0,180).Quaternion();
				//}
				UpVector = FVector(0,0,1);
				FVector NormalVector = Normal;

				FVector RotationAxis = FVector::CrossProduct(UpVector, NormalVector);
				RotationAxis.Normalize();

				float DotProduct = FVector::DotProduct(UpVector, NormalVector);
				float RotationAngle = acosf(DotProduct);

				Quat = FQuat(RotationAxis, RotationAngle);

				NewQuat = Quat; //;
				
				NewQuat*= FRotator(0, GetScrollRotateValue(), 0).Quaternion();
			}else{
				NewQuat = HitResult.GetActor()->GetActorRotation().Quaternion();
				NewQuat*= FRotator(0,0,0).Quaternion();
				if(UpsideDown){
					NewQuat*= FRotator(0,0,180).Quaternion();
				}
				NewQuat*= FRotator(0, GetScrollRotateValue(), 0).Quaternion();
			}
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

void AFINModularIndicatorPoleHolo::CheckValidFloor() {}

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
