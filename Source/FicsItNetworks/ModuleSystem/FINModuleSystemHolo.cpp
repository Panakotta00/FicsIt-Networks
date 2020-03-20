#include "FINModuleSystemHolo.h"

#include "FINModuleSystemModule.h"
#include "FINModuleSystemPanel.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGConstructDisqualifier.h"

AFINModuleSystemHolo::AFINModuleSystemHolo() {}

AFINModuleSystemHolo::~AFINModuleSystemHolo() {}

AActor* AFINModuleSystemHolo::Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) {
	FRotator rotation = GetActorRotation();
	FVector location = GetActorLocation();

	auto a = Super::Construct(childs, constructionID);

	Cast<IFINModuleSystemModule>(a)->setPanel(Snapped, (int) SnappedLoc.X, (int) SnappedLoc.Y, (int) SnappedRot);
	
	return a;
}

bool AFINModuleSystemHolo::checkSpace(FVector min, FVector max) {
	if (min.X < 0 || min.X >= Snapped->PanelHeight || min.Y < 0 || min.Y >= Snapped->PanelWidth) return false;
	if (max.X < 0 || max.X >= Snapped->PanelHeight || max.Y < 0 || max.Y >= Snapped->PanelWidth) return false;
	for (int x = (int) min.X; x <= max.X; x++) for (int y = (int) min.Y; y <= max.Y; y++) {
		if (Snapped->grid[x][y]) return false;
	}
	return true;
}

FVector AFINModuleSystemHolo::getModuleSize() {
	auto o = Cast<IFINModuleSystemModule>(GetDefault<AFGBuildable>(mBuildClass));
	int w, h;
	o->getModuleSize(w, h);
	return FVector((float) h, (float) w, 0);
}

bool AFINModuleSystemHolo::IsValidHitResult(const FHitResult& hit) const {
	auto r = GetScrollValue(EHologramScrollMode::HSM_ROTATE);

	UFINModuleSystemPanel* panel = Cast<UFINModuleSystemPanel>(hit.Component.Get());
	while (IsValid(panel) && !panel->IsA<UFINModuleSystemPanel>()) panel = (UFINModuleSystemPanel*)panel->GetOwner();

	return panel;
}

bool AFINModuleSystemHolo::TrySnapToActor(const FHitResult& hitResult) {
	UFINModuleSystemPanel* panel = Cast<UFINModuleSystemPanel>(hitResult.Component.Get());
	while (IsValid(panel) && !panel->IsA<UFINModuleSystemPanel>()) panel = (UFINModuleSystemPanel*)panel->GetOwner();

	if (!IsValid(panel)) {
		Snapped = nullptr;
		return false;
	}
	Snapped = panel;

	auto loc = UKismetMathLibrary::InverseTransformLocation(Snapped->GetComponentToWorld(), hitResult.Location);
	SnappedLoc = loc;
	SnappedLoc = SnappedLoc / 10.0;
	SnappedLoc.X = floor(SnappedLoc.X);
	SnappedLoc.Y = floor(SnappedLoc.Y);
	SnappedLoc.Z = 0;

	FVector min, max;
	switch (GetScrollRotateValue() % 40) {
	case 0:
		UFINModuleSystemPanel::getModuleSpace(SnappedLoc, SnappedRot = 0, getModuleSize(), min, max);
		break;
	case -30:
	case 10:
		UFINModuleSystemPanel::getModuleSpace(SnappedLoc, SnappedRot = 1, getModuleSize(), min, max);
		break;
	case -20:
	case 20:
		UFINModuleSystemPanel::getModuleSpace(SnappedLoc, SnappedRot = 2, getModuleSize(), min, max);
		break;
	case -10:
	case 30:
		UFINModuleSystemPanel::getModuleSpace(SnappedLoc, SnappedRot = 3, getModuleSize(), min, max);
		break;
	}
	bIsValid = checkSpace(min, max);
	if (bIsValid) {
		bIsValid = false;
		for (auto& allowed : Snapped->AllowedModules)
			if (mBuildClass->IsChildOf(allowed)) 
				bIsValid = true;
	}
	return true;
}

void AFINModuleSystemHolo::SetHologramLocationAndRotation(const FHitResult& hit) {
	auto loc = SnappedLoc;
	switch (SnappedRot) {
	case 0:
		break;
	case 1:
		loc = loc + FVector{1, 0, 0};
		break;
	case 2:
		loc = loc + FVector{1, 1, 0};
		break;
	case 3:
		loc = loc + FVector{0, 1, 0};
		break;
	}
	loc = loc * 10.0;

	FRotator rot = {0, SnappedRot * 90.0f, 0.0f};
	rot = UKismetMathLibrary::TransformRotation(Snapped->GetComponentToWorld(), rot);
	loc = UKismetMathLibrary::TransformLocation(Snapped->GetComponentToWorld(), loc);
	SetActorLocationAndRotation(loc, rot);
}

void AFINModuleSystemHolo::CheckValidPlacement() {
	if (!bIsValid) AddConstructDisqualifier(UFGCDInvalidPlacement::StaticClass());
}
