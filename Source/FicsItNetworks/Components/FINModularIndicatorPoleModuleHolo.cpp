#include "FINModularIndicatorPoleModuleHolo.h"
#include "FINModularIndicatorPoleModule.h"
#include "FINModularIndicatorPole.h"

AFINModularIndicatorPoleModuleHolo::AFINModularIndicatorPoleModuleHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	//mMaxZoopAmount = 10;
	//mPlacementRequirements = EFactoryBuildingPlacementRequirements::FBPR_MustSnap;
	//mBuildModeCategory = EHologramBuildModeCategory::HBMC_ActorClass;
	//mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_Backward + (uint8)EHologramZoopDirections::HZD_Forward + (uint8)EHologramZoopDirections::HZD_Left;
}

void AFINModularIndicatorPoleModuleHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFINModularIndicatorPoleModuleHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFINModularIndicatorPoleModuleHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

bool AFINModularIndicatorPoleModuleHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	return true;
}

int32 AFINModularIndicatorPoleModuleHolo::GetBaseCostMultiplier() const {
	return 1;
}

bool AFINModularIndicatorPoleModuleHolo::IsValidHitResult(const FHitResult& hitResult) const {
	AActor* Component = hitResult.GetActor();
	if(const AFINModularIndicatorPoleModule* Temp = Cast<AFINModularIndicatorPoleModule>(Component); IsValid(Temp)) {
		return !IsValid(Temp->NextChild);
	}
	if(const AFINModularIndicatorPole* Temp = Cast<AFINModularIndicatorPole>(Component); IsValid(Temp)) {
		return !IsValid(Temp->ChildModule);
	}
	return false;
}

void AFINModularIndicatorPoleModuleHolo::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	Super::SetHologramLocationAndRotation(hitResult);
	
	if (SnappedObject.IsValid()) {
		SetActorLocation(GetAttachPoint(SnappedObject.Get()));
		SetActorRotation(SnappedObject->GetActorRotation());
	}
}

void AFINModularIndicatorPoleModuleHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);

	if(AFINModularIndicatorPoleModule* Module = Cast<AFINModularIndicatorPoleModule>(inBuildable); IsValid(Module)) {
		if (SnappedObject.IsValid()) {
			Module->Parent = Cast<AFINModularIndicatorPoleModule>(SnappedObject.Get());
			if(AFINModularIndicatorPoleModule* SnappedModule = Cast<AFINModularIndicatorPoleModule>(SnappedObject); IsValid(SnappedModule)) {
				SnappedModule->NextChild = Module;
			}else if(AFINModularIndicatorPole* SnappedPole = Cast<AFINModularIndicatorPole>(SnappedObject); IsValid(SnappedPole)) {
				SnappedPole->ChildModule = Module;
			}
		}
	}
}

void AFINModularIndicatorPoleModuleHolo::CheckValidFloor() {
	if (!bSnapped) Super::CheckValidFloor();
}

FVector AFINModularIndicatorPoleModuleHolo::GetAttachPoint(AFGBuildable* Object) const {
	if(const AFINModularIndicatorPoleModule* Temp = Cast<AFINModularIndicatorPoleModule>(Object); IsValid(Temp)) {
		return Object->GetActorLocation() + Temp->GetActorRotation().RotateVector(Temp->ModuleConnectionPoint);
	}
	if(const AFINModularIndicatorPole* Temp = Cast<AFINModularIndicatorPole>(Object); IsValid(Temp)) {
		return Object->GetActorLocation() + Temp->GetActorRotation().RotateVector(Temp->ModuleConnectionPoint);
	}
	return Object->GetActorLocation();
}

bool AFINModularIndicatorPoleModuleHolo::TrySnapToActor(const FHitResult& hitResult) {
	if (!bSnapped) {
		AFGBuildable* Snapped = nullptr;
		if (AFINModularIndicatorPole* Pole = Cast<AFINModularIndicatorPole>(hitResult.Actor.Get()); Pole && !IsValid(Pole->ChildModule)) {
			Snapped = Pole;
		}else if(AFINModularIndicatorPoleModule* Module = Cast<AFINModularIndicatorPoleModule>(hitResult.Actor.Get()); Module && !IsValid(Module->NextChild)) {
			Snapped = Module;
		}
		if(IsValid(Snapped)) {
			SnappedObject = Snapped;
			SetActorLocation(GetAttachPoint(SnappedObject.Get()));
			SetActorRotation(SnappedObject->GetActorRotation());
			return true;
		}
		SnappedObject = nullptr;
	}
	return false;
}

//bool AFINModularIndicatorPoleModuleHolo::CanBeZooped() const {
//	return true;
//	//return Super::CanBeZooped();
//}
//
//void AFINModularIndicatorPoleModuleHolo::ConstructZoop(TArray<AActor*>& Actors) {
//	Super::ConstructZoop(Actors);
//}
//
//void AFINModularIndicatorPoleModuleHolo::GetSupportedBuildModes_Implementation(
//	TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& MovieSceneBlendses) const {
//	Super::GetSupportedBuildModes_Implementation(MovieSceneBlendses);
//}

