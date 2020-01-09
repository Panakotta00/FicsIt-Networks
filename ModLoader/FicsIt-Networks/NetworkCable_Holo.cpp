#include "stdafx.h"
#include "NetworkCable_Holo.h"

#include <mod/ModFunctions.h>
#include <assets/AssetFunctions.h>
#include <assets/FObjectSpawnParameters.h>
#include <game/Global.h>
#include <assets/BPInterface.h>

#include <util/Objects/UClass.h>
#include <util/Objects/UProperty.h>

#include "ComponentUtility.h"
#include "NetworkAdapter.h"

using namespace SML;
using namespace SML::Mod;
using namespace SML::Objects;

enum class EOutlineColor : uint8_t {
	OC_NONE = 0,
	OC_USABLE = 252,
	OC_HOLOGRAM = 253,
	OC_RED = 254,
	OC_MAX = 4
};

bool ANetworkCable_Holo::multiPlacement() {
	if (from.ptr == snapped.ptr) return false;
	if (from.v && snapped.v) return isValid() && isSnappedValid();
	if (!snapped.v) {
		updateMeshes();
		return false;
	}
	if (!isSnappedValid()) return false;

	from = snapped;

	onBeginSnap(from, false);

	return false;
}

UNetworkConnector* setupSnapped(SnappedInfo s) {
	if (s.isConnector) return s.c();
	
	SDK::FRotator rotation = s.f()->K2_GetActorRotation();
	SDK::FVector location = s.pos;
	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;
	auto a = (ANetworkAdapter*)::call<&SML::Objects::UWorld::SpawnActor>((SML::Objects::UWorld*)*SDK::UWorld::GWorld, (SDK::UClass*)ANetworkAdapter::staticClass(), &location, &rotation, (FActorSpawnParameters*) &spawnParams);
	a->parent = s.f();
	((SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass()->CreateDefaultObject())->FinishSpawningActor(a, s.f()->GetTransform());
	a->K2_SetActorRotation(rotation, false);
	a->K2_SetActorLocation(location, false, true, nullptr);
	return a->connector;
}


SDK::AActor * ANetworkCable_Holo::constructFinal(TArray<AActor*>& childs) {
	auto c1 = setupSnapped(snapped);
	auto c2 = setupSnapped(from);
	
	SDK::FRotator rotation = SDK::FRotator();
	SDK::FVector location = c1->K2_GetComponentToWorld().Translation;
	SDK::FVector scale;
	scale.X = scale.Y = scale.Z = 1.0;
	SDK::FQuat rot;
	rot.W = 0;
	rot.X = 0;
	rot.Y = 0;
	rot.Z = 0;

	FActorSpawnParameters spawnParams;
	spawnParams.bDeferConstruction = true;

	auto a = (AActor*)::call<&SML::Objects::UWorld::SpawnActor>((SML::Objects::UWorld*)*SDK::UWorld::GWorld, this->mBuildableClass, &location, &rotation, (FActorSpawnParameters*)&spawnParams);
	
	*((Objects::UClass*)mBuildableClass)->findField<UProperty>("Connector1")->getValue<UNetworkConnector*>(a) = c1;
	*((Objects::UClass*)mBuildableClass)->findField<UProperty>("Connector2")->getValue<UNetworkConnector*>(a) = c2;

	SDK::FTransform t = a->GetTransform();
	t.Rotation = rot;
	t.Scale3D = scale;
	t.Translation = location;

	((SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass()->CreateDefaultObject())->FinishSpawningActor(a, t);

	return a;
}

TArray<SDK::FItemAmount> ANetworkCable_Holo::getCost(bool includeChildren) {
	static SDK::UClass* c = nullptr;
	if (!c) c = (SDK::UClass*) Functions::loadObjectFromPak(L"/Game/FactoryGame/Resource/Parts/Wire/Desc_Wire.Desc_Wire_C");
	TArray<SDK::FItemAmount> items;
	
	if (snapped.v && from.v) {
		SDK::FItemAmount amount;
		auto len = (snapped.pos - from.pos).length();
		amount.amount = (int) ceilf(len / 100.0f);
		amount.ItemClass = c;
		items.add(amount);
	}

	return items;
}

bool ANetworkCable_Holo::isValidHit(const SDK::FHitResult & hit) {
	SDK::UClass* clazz = (SDK::UClass*)Paks::ClassBuilder<UNetworkConnector>::staticClass();
	auto obj = UObject::GetObjectCasted<AActor>(FWeakObjectPtr(hit.Actor).index);
	
	auto c = UComponentUtility::getNetworkConnectorFromHit(hit);
	if (c) {
		snapped = {true, true, c, obj, c->K2_GetComponentToWorld().Translation};
		return true;
	}

	if (obj->IsA(SDK::AFGBuildableFactory::StaticClass())) {
		TArray<SDK::UActorComponent*> cons = obj->GetComponentsByClass(SDK::UFGPowerConnectionComponent::StaticClass());
		float dist = -1.0f;
		SDK::USceneComponent* con = nullptr;
		for (auto c : cons) {
			float d = ((FVector)((SDK::USceneComponent*)c)->K2_GetComponentToWorld().Translation - (FVector)hit.Location).length();
			if (dist < 0.0f || dist > d) {
				con = (SDK::USceneComponent*)c;
				dist = d;
			}
		}
		if (con) {
			snapped = {true, false, obj, obj, con->K2_GetComponentToWorld().Translation};
			return true;
		}
	}
	
	for (auto entry : ANetworkAdapter::settings) {
		auto setting = entry.second;
		auto clazz = entry.first;

		if (obj->IsA(clazz)) {
			auto t = ((SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass()->CreateDefaultObject())->TransformLocation(obj->GetTransform(), setting.loc);
			snapped = { true, false, obj, obj, t };
			

			return true;
		}
	}

	snapped.v = false;
	snapped.ptr = nullptr;

	return false;
}

bool ANetworkCable_Holo::isSnappedValid() {
	return snapped.v && (!snapped.isConnector || (snapped.c()->cables.size() < snapped.c()->maxCables));
}

void ANetworkCable_Holo::setHoloLocAndRot(SDK::FHitResult * hit) {
	this->RootComponent->K2_SetWorldLocation((snapped.v) ? snapped.pos : hit->Location, false, true, nullptr);

	updateMeshes();

	if (snapped.ptr != oldSnapped.ptr) {
		onEndSnap(oldSnapped);
		onBeginSnap(snapped, isSnappedValid() && (snapped.ptr != from.ptr) && (!from.v || (isValid())));

		oldSnapped = snapped;
	}
}

void ANetworkCable_Holo::onInvalidHolo() {
	onEndSnap(snapped);
	onEndSnap(oldSnapped);
	snapped.v = oldSnapped.v = false;
	snapped.ptr = oldSnapped.ptr = nullptr;
}

void ANetworkCable_Holo::onBeginSnap(SnappedInfo a, bool isValid) {
	if (a.v) {
		auto o = a.actor;
		if (o) Functions::getPlayerCharacter()->GetOutline()->ShowOutline(o,(SDK::EOutlineColor)(isValid ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_RED));
		this->Client_PlaySnapSound();
	}
}

void ANetworkCable_Holo::onEndSnap(SnappedInfo a) {
	static SDK::UFGBlueprintFunctionLibrary* bpf = nullptr;
	if (!bpf) bpf = (SDK::UFGBlueprintFunctionLibrary*)SDK::UFGBlueprintFunctionLibrary::StaticClass()->CreateDefaultObject();

	if (a.v) {
		auto* o = a.actor;
		if (o) Functions::getPlayerCharacter()->GetOutline()->HideOutline();
	}
}

void ANetworkCable_Holo::updateMeshes() {
	if (!snapped.v || !from.v || snapped.ptr == from.ptr) {
		cable->SetVisibility(false, true);
		if (mInvalidPlacementMaterial) con->SetMaterial(0, mInvalidPlacementMaterial);
		return;
	}
	cable->SetVisibility(true, true);

	float offset = 250.0;
	SDK::FVector start;
	start.X = start.Y = start.Z = 0;
	SDK::FVector end = ((SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass()->CreateDefaultObject())->InverseTransformLocation(RootComponent->K2_GetComponentToWorld(), from.pos);
	SDK::FVector start_t = end;
	start_t.Z -= offset;
	SDK::FVector end_t = end;
	end_t.Z += offset;
	
	cable->SetStartAndEnd(start, start_t, end, end_t, true);

	if (isValid() && isSnappedValid()) {
		cable->SetMaterial(0, mValidPlacementMaterial);
		con->SetMaterial(0, mValidPlacementMaterial);
	} else {
		cable->SetMaterial(0, mInvalidPlacementMaterial);
		con->SetMaterial(0, mInvalidPlacementMaterial);
	}
}

bool ANetworkCable_Holo::isValid() {
	if (!from.v || !snapped.v) return false;
	TArray<SDK::AActor*> overlaps;
	cable->GetOverlappingActors(nullptr, (SDK::TArray<SDK::AActor*>*)&overlaps);
	for (auto actor : overlaps) {
		if (from.actor != actor && (!from.isConnector || (SDK::AActor*)from.c()->Outer != actor) && snapped.actor != actor && (!snapped.isConnector || (SDK::AActor*)snapped.c()->Outer != actor) && this != actor) {
			return false;
		}
	}
	return ((from.isConnector && snapped.isConnector) ? !from.c()->isConnected(snapped.c()) : true) && (from.pos - snapped.pos).length() <= 10000.0f;
}

void ANetworkCable_Holo::constructOI(Paks::FObjectInitializer& oi) {
	Paks::ClassBuilder<ANetworkCable_Holo>::construct(oi);
	auto self = (UObject*)oi.obj;
}

void ANetworkCable_Holo::construct() {
	static SDK::UStaticMesh* mesh = nullptr;
	if (!mesh) mesh = (SDK::UStaticMesh*) Functions::loadObjectFromPak(SDK::UStaticMesh::StaticClass(), L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable");
	static SDK::UMaterialInterface* mat = nullptr;
	if (!mat) mat = (SDK::UMaterialInterface*) Functions::loadObjectFromPak(SDK::UMaterialInterface::StaticClass(), L"/Game/FactoryGame/Equipment/BuildGun/Material/HologramColor_Inst.HologramColor_Inst");
	static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");
	static SDK::UStaticMesh* meshCon = nullptr;
	if (!meshCon) meshCon = (SDK::UStaticMesh*)Functions::loadObjectFromPak(SDK::UStaticMesh::StaticClass(), L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/Mesh_NetworkConnector_Holo.Mesh_NetworkConnector_Holo");
	static void(*regComp)(SDK::AActor*, SDK::UActorComponent*);
	if (!regComp) regComp = (void(*)(SDK::AActor*, SDK::UActorComponent*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AActor::FinishAndRegisterComponent");

	((bool(ANetworkCable_Holo::**)())Vtable)[0xC9] = &ANetworkCable_Holo::multiPlacement;
	((AActor*(ANetworkCable_Holo::**)(TArray<AActor*>&))Vtable)[0xD0] = &ANetworkCable_Holo::constructFinal;
	((void(ANetworkCable_Holo::**)(SDK::FHitResult*))Vtable)[0xC7] = &ANetworkCable_Holo::setHoloLocAndRot;
	((bool(ANetworkCable_Holo::**)(const SDK::FHitResult&))Vtable)[0xC3] = &ANetworkCable_Holo::isValidHit;
	((void(ANetworkCable_Holo::**)())Vtable)[0xC8] = &ANetworkCable_Holo::onInvalidHolo;
	((TArray<SDK::FItemAmount>(ANetworkCable_Holo::**)(bool))Vtable)[0xD1] = &ANetworkCable_Holo::getCost;
	cable = nullptr;
	con = nullptr;
	snapped = SnappedInfo();
	oldSnapped = SnappedInfo();
	from = SnappedInfo();
	this->mMaxPlacementFloorAngle = 90.0f;

	auto self = (Objects::UObject*)this;
	cable = self->createDefaultSubobjectSDK<SDK::USplineMeshComponent>(L"Cable");
	cable->SetMobility(SDK::EComponentMobility::Movable);
	setupAttach(cable, RootComponent, FName());
	cable->SetStaticMesh(mesh);
	cable->ForwardAxis = SDK::ESplineMeshAxis::Z;
	cable->SetCollisionEnabled(SDK::ECollisionEnabled::QueryOnly);
	cable->SetCollisionObjectType(SDK::ECollisionChannel::ECC_GameTraceChannel3);
	cable->SetCollisionResponseToAllChannels(SDK::ECollisionResponse::ECR_Overlap);
	cable->SetAllUseCCD(true);

	SDK::FVector loc;
	loc.X = loc.Y = loc.Z = 0.0;
	SDK::FVector scl;
	scl.X = scl.Y = scl.Z = 1.0;
	con = self->createDefaultSubobjectSDK<SDK::USplineMeshComponent>(L"ConnectorMesh");
	con->SetMobility(SDK::EComponentMobility::Movable);
	setupAttach(con, RootComponent, FName());
	con->SetStaticMesh(meshCon);
	updateMeshes();
}

void ANetworkCable_Holo::destruct() {

}
