#include "Components/FINVehicleScanner.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGVehicle.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AFINVehicleScanner::AFINVehicleScanner() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>(TEXT("NetworkConnector"));
	NetworkConnector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NetworkConnector->SetIsReplicated(true);
	StaticMesh = CreateDefaultSubobject<UFGColoredInstanceMeshProxy>(TEXT("StaticMesh"));
	StaticMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	LampMesh= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
	LampMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleCollision"));
	VehicleCollision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINVehicleScanner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINVehicleScanner, ScanColor);
	DOREPLIFETIME(AFINVehicleScanner, Intensity);
}

void AFINVehicleScanner::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (bColorChanged) {
		bColorChanged = false;
		Client_OnColorChanged();
	}
}

void AFINVehicleScanner::BeginPlay() {
	Super::BeginPlay();
	if (LampMesh->GetMaterials().Num() > 0) {
		int matIdx = LampMesh->GetMaterialIndex("CustomEmissive_Scanner");
		LightMaterialInstance = LampMesh->CreateDynamicMaterialInstance(matIdx);
		LampMesh->SetMaterial(matIdx, LightMaterialInstance);
	}
	UpdateColor();
}

void AFINVehicleScanner::NotifyActorBeginOverlap(AActor* OtherActor) {
	Super::NotifyActorBeginOverlap(OtherActor);
	AFGVehicle* Vehicle = Cast<AFGVehicle>(OtherActor);
	if (Vehicle) {
		netSig_OnVehicleEnter(Vehicle);
		LastVehicle = Vehicle;
	}
}

void AFINVehicleScanner::NotifyActorEndOverlap(AActor* OtherActor) {
	Super::NotifyActorEndOverlap(OtherActor);

	TArray<AActor*> Actors;
	GetOverlappingActors(Actors);
	
	AFGVehicle* Vehicle = Cast<AFGVehicle>(OtherActor);
	if (Vehicle) {
		netSig_OnVehicleExit(Vehicle);
		bool found = false;
		for (AActor* Actor : Actors) {
			if (Actor->IsA<AFGVehicle>()) {
				found = true;
				break;
			}
		}
		if (!found) LastVehicle = nullptr;
	}
}

UObject* AFINVehicleScanner::GetSignalSenderOverride_Implementation() {
	return this;
}

void AFINVehicleScanner::Client_OnColorChanged_Implementation() {
	UpdateColor();
}

void AFINVehicleScanner::UpdateColor_Implementation() {
	if (LightMaterialInstance) {
		LightMaterialInstance->SetVectorParameterValue("Color", ScanColor);
		LightMaterialInstance->SetScalarParameterValue("Intensity", Intensity);
	}
}

void AFINVehicleScanner::netFunc_setColor(float r, float g, float b, float e) {
	ScanColor = FLinearColor(FMath::Clamp(r, 0.0f, 1.0f), FMath::Clamp(g, 0.0f, 1.0f), FMath::Clamp(b, 0.0f, 1.0f));
	Intensity = FMath::Clamp(e, 0.0f, 5.0f);
	bColorChanged = true;
}

void AFINVehicleScanner::netFunc_getColor(float& r, float& g, float& b, float& e) {
	r = ScanColor.R;
	g = ScanColor.G;
	b = ScanColor.B;
	e = Intensity;
}

AFGVehicle* AFINVehicleScanner::netFunc_getLastVehicle() {
	return LastVehicle;
}

void AFINVehicleScanner::netSig_OnVehicleExit_Implementation(AFGVehicle* Vehicle) {}
void AFINVehicleScanner::netSig_OnVehicleEnter_Implementation(AFGVehicle* Vehicle) {}

