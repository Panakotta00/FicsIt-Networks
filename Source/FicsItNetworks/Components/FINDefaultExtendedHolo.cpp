#include "FINDefaultExtendedHolo.h"

#include "Kismet/KismetMathLibrary.h"


UBuildMode_Default::UBuildMode_Default() {
	mDisplayName = FText::FromString("Auto");
}

UBuildMode_Snapped45::UBuildMode_Snapped45() {
	mDisplayName = FText::FromString("Default, 45°");
}

UBuildMode_FreePlacement::UBuildMode_FreePlacement() {
	mDisplayName = FText::FromString("Free");
}


AFINDefaultExtendedHolo::AFINDefaultExtendedHolo() {
	mBuildModeAuto = UBuildMode_Default::StaticClass();
	mDefaultBuildMode = mBuildModeAuto;
	mBuildMode45 = UBuildMode_Snapped45::StaticClass();
	mBuildModeFree = UBuildMode_FreePlacement::StaticClass();
}

void AFINDefaultExtendedHolo::GetSupportedBuildModes_Implementation(
	TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const {
	Super::GetSupportedBuildModes_Implementation(out_buildmodes);
	out_buildmodes.Add(mBuildModeAuto);
	out_buildmodes.Add(mBuildMode45);
	out_buildmodes.Add(mBuildModeFree);
}

void AFINDefaultExtendedHolo::SetHologramLocationAndRotation(const FHitResult& HitResult) {
	//AFGBuildableHologram::SetHologramLocationAndRotation(hitResult);
	if(mCurrentBuildMode == mBuildModeAuto) {
		AFGBuildableHologram::SetHologramLocationAndRotation(HitResult);
	}else if(mCurrentBuildMode == mBuildMode45 || mCurrentBuildMode == mBuildModeFree) {
		FVector Normal = HitResult.ImpactNormal;

		FQuat Quat;

		FVector LocalNormal = HitResult.GetActor()->GetTransform().InverseTransformVectorNoScale(Normal);
		Quat = HitResult.GetActor()->GetTransform().GetRotation();
		FVector UpVector = Quat.GetUpVector();
		Quat *= FQuat(FVector::CrossProduct(UpVector, LocalNormal), FMath::Acos(FVector::DotProduct(LocalNormal, UpVector)));
		
		FQuat NewQuat = Quat; //;
		if(mCurrentBuildMode == mBuildModeFree) {
			NewQuat*= FRotator(0, GetScrollRotateValue(), 0).Quaternion();
			const FVector Res = HitResult.ImpactPoint;
			//resDiffPos = FVector::CrossProduct();
			SetActorLocationAndRotation(Res, NewQuat.Rotator());
		}else {
			const auto Location = HitResult.GetActor()->GetActorLocation();
			const auto Rotation = HitResult.GetActor()->GetActorRotation();
			const auto VectorInActorLocalSpace = Rotation.UnrotateVector(HitResult.ImpactPoint - Location);
			const FVector GridPos = VectorInActorLocalSpace.GridSnap(100);
			const FVector ResDiffPos = GridPos - VectorInActorLocalSpace;
			const FVector ResDiffPosR = Rotation.RotateVector(ResDiffPos);
			const FVector Res = HitResult.ImpactPoint + ResDiffPosR;
			
			NewQuat*= FRotator(0, (GetScrollRotateValue() / 10 * 45) % 360, 0).Quaternion();
			//resDiffPos = FVector::CrossProduct();
			SetActorLocationAndRotation(Res, NewQuat.Rotator());
		}
	}
}
