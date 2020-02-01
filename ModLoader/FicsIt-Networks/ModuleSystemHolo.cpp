#include "stdafx.h"
#include "ModuleSystemHolo.h"
#include <mod/ModFunctions.h>
#include <assets/AssetFunctions.h>
#include <assets/FObjectSpawnParameters.h>
#include <game/Global.h>
#include <util/objects/SmartPointer.h>

using namespace SML;
using namespace SML::Mod;
using namespace SML::Objects;

SDK::AActor*(AModuleSystemHolo::* parentConstructFinal)(SML::Objects::TArray<SDK::AActor*>&) = nullptr;

void AModuleSystemHolo::construct() {
	if (!parentConstructFinal) parentConstructFinal = ((AActor*(AModuleSystemHolo::**)(TArray<AActor*>&))Vtable)[0xD0];
	((AActor*(AModuleSystemHolo::**)(TArray<AActor*>&))Vtable)[0xD0] = &AModuleSystemHolo::constructFinal;
	((void(AModuleSystemHolo::**)(SDK::FHitResult*))Vtable)[0xC7] = &AModuleSystemHolo::setHoloLocAndRot;
	((bool(AModuleSystemHolo::**)(const SDK::FHitResult&))Vtable)[0xC3] = &AModuleSystemHolo::isValidHit;
	(((void(AModuleSystemHolo::**)())Vtable)[0xD4]) = &AModuleSystemHolo::checkValid;
	snapped = nullptr;
}

void AModuleSystemHolo::destruct() {}

SDK::AActor* AModuleSystemHolo::constructFinal(SML::Objects::TArray<SDK::AActor*>& childs) {
	SDK::FRotator rotation = this->K2_GetActorRotation();
	SDK::FVector location = this->K2_GetActorLocation();

	auto a = (this->*parentConstructFinal)(childs);

	struct { UModuleSystemPanel* p; int x; int y; int r; } p {snapped, (int) snappedLoc.X, (int) snappedLoc.Y, (int) snappedRot};
	((Objects::UObject*)a)->findFunction(L"setPanel")->invoke((Objects::UObject*)a, &p);

	return a;
}

bool AModuleSystemHolo::checkSpace(FVector min, FVector max) {
	if (min.X < 0 || min.X >= snapped->panelHeight || min.Y < 0 || min.Y >= snapped->panelWidth) return false;
	if (max.X < 0 || max.X >= snapped->panelHeight || max.Y < 0 || max.Y >= snapped->panelWidth) return false;
	for (int x = (int) min.X; x <= max.X; x++) for (int y = (int) min.Y; y <= max.Y; y++) {
		if (snapped->grid[x][y]) return false;
	}
	return true;
}

FVector AModuleSystemHolo::getModuleSize() {
	auto o = (Objects::UObject*)mBuildableClass->CreateDefaultObject();
	struct Params {
		int w, h;
	};
	Params p;
	o->findFunction(L"getModuleSize")->invoke(o, &p);
	return FVector((float) p.h, (float) p.w, 0);
}

bool AModuleSystemHolo::isValidHit(const SDK::FHitResult & hit) {
	static int(*getR)(AModuleSystemHolo*, SDK::EHologramScrollMode) = nullptr;
	if (!getR) getR = (int(*)(AModuleSystemHolo*, SDK::EHologramScrollMode)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGHologram::GetScrollValue");
	auto r = getR(this, SDK::EHologramScrollMode::HSM_ROTATE);

	UModuleSystemPanel* p = (UModuleSystemPanel*)((FWeakObjectPtr)hit.component).get();
	while (p && !p->IsA((SDK::UClass*)UModuleSystemPanel::staticClass())) p = (UModuleSystemPanel*)p->AttachParent;
	if (!p) {
		snapped = nullptr;
		return false;
	}
	snapped = p;

	auto loc = ((SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass()->CreateDefaultObject())->InverseTransformLocation(snapped->K2_GetComponentToWorld(), hit.Location);
	snappedLoc = loc;
	snappedLoc = snappedLoc / 10.0;
	snappedLoc.X = std::floor(snappedLoc.X);
	snappedLoc.Y = std::floor(snappedLoc.Y);
	snappedLoc.Z = 0;

	FVector min, max;
	switch (r % 40) {
	case 0:
		UModuleSystemPanel::getModuleSpace(snappedLoc, snappedRot = 0, getModuleSize(), min, max);
		break;
	case -30:
	case 10:
		UModuleSystemPanel::getModuleSpace(snappedLoc, snappedRot = 1, getModuleSize(), min, max);
		break;
	case -20:
	case 20:
		UModuleSystemPanel::getModuleSpace(snappedLoc, snappedRot = 2, getModuleSize(), min, max);
		break;
	case -10:
	case 30:
		UModuleSystemPanel::getModuleSpace(snappedLoc, snappedRot = 3, getModuleSize(), min, max);
		break;
	}
	isValid = checkSpace(min, max);
	if (isValid) {
		isValid = false;
		for (auto& allowed : snapped->allowedModules)
			if (((Objects::UClass*)this->mBuildableClass)->isChild(allowed)) 
				isValid = true;
	}
	return true;
}

void AModuleSystemHolo::setHoloLocAndRot(SDK::FHitResult * hit) {
	auto loc = snappedLoc;
	switch (snappedRot) {
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

	SDK::FRotator rot = {0,snappedRot * 90.0f,0};
	rot = ((SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass()->CreateDefaultObject())->TransformRotation(snapped->K2_GetComponentToWorld(), rot);
	loc = ((SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass()->CreateDefaultObject())->TransformLocation(snapped->K2_GetComponentToWorld(), loc);
	this->K2_SetActorLocationAndRotation(loc, rot, false, true, nullptr);
}

void AModuleSystemHolo::checkValid() {
	void(*add)(AModuleSystemHolo*, SDK::UClass*) = nullptr;
	if (!add) add = (void(*)(AModuleSystemHolo*, SDK::UClass*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGHologram::AddConstructDisqualifier");
	if (!isValid) add(this, SDK::UFGCDInvalidPlacement::StaticClass());
}
