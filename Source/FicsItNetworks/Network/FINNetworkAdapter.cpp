#include "FINNetworkAdapter.h"

#include "FGPowerConnectionComponent.h"
#include "FGItemPickup_Spawnable.h"

#include "Components/SceneComponent.h"

std::vector<std::pair<UClass*, FFINAdapterSettings>> AFINNetworkAdapter::settings = std::vector<std::pair<UClass*, FFINAdapterSettings>>();

void AFINNetworkAdapter::RegistererAdapterSetting(UClass* clazz, FFINAdapterSettings settings) {
	AFINNetworkAdapter::settings.push_back({clazz, settings});
}

void AFINNetworkAdapter::RegistererAdapterSetting(FString BPPath, FFINAdapterSettings settings) {
	UClass* clazz = LoadObject<UClass>(nullptr, *BPPath);
	RegistererAdapterSetting(clazz, settings);
}

void AFINNetworkAdapter::RegisterAdapterSettings() {
	// init adapter settings
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk1/Build_StorageContainerMk1.Build_StorageContainerMk1_C")), FFINAdapterSettings{FVector(290,0,400), FRotator(0,-90,0), true, 1});
}

AFINNetworkAdapter::AFINNetworkAdapter() {
	UStaticMesh* networkAdapterMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsIt-Networks/Network/Mesh_Adapter.Mesh_Adapter"));
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"Root");

	Connector = CreateDefaultSubobject<UFINNetworkConnector>(L"Connector");
	Connector->SetupAttachment(RootComponent);
	
	ConnectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(L"StaticMesh");
	ConnectorMesh->SetHiddenInGameSML(true, true);
	ConnectorMesh->SetupAttachment(RootComponent, FName());
	ConnectorMesh->SetStaticMesh(networkAdapterMesh);

	Connector->MaxCables = 1;
}

AFINNetworkAdapter::~AFINNetworkAdapter() {}

void AFINNetworkAdapter::BeginPlay() {
	Super::BeginPlay();

	if (!IsValid(Parent)) {
		for (AFINNetworkCable* cable : Connector->Cables) {
			TArray<FInventoryStack> refund;
			cable->Execute_GetDismantleRefund(cable, refund);
			for (FInventoryStack& stack : refund) AFGItemPickup_Spawnable::AddItemToWorldStackAtLocation(GetWorld(), stack, cable->GetActorLocation(), cable->GetActorRotation());
			cable->Destroy();
		}
		Destroy();
		return;
	}
	
	Connector->Merged.Add(Parent);

	Attachment = NewObject<UFINNetworkAdapterReference>((Parent) ? Parent : nullptr);
	Attachment->Ref = this;
	Attachment->RegisterComponent();

	if (Parent->IsA<AFGBuildableFactory>()) {
		auto cons = Parent->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
		float dist = -1.0f;
		USceneComponent* con = nullptr;
		for (auto c : cons) {
			float d = (Cast<USceneComponent>(c)->GetComponentToWorld().GetTranslation() - this->GetActorLocation()).Size();
			if (dist < 0.0f || dist > d) {
				con = Cast<USceneComponent>(c);
				dist = d;
			}
		}
		if (con) {
			SetActorLocationAndRotation(con->GetComponentLocation(), con->GetComponentRotation());
			return;
		}
	}

	for (auto setting_entry : settings) {
		auto clazz = setting_entry.first;
		auto setting = setting_entry.second;
		if (!Parent->IsA(clazz)) continue;
		FVector pos = Parent->GetActorTransform().TransformPosition(Parent->K2_GetActorLocation());
		SetActorLocationAndRotation(pos, Parent->GetActorRotation());
		FHitResult res;
		ConnectorMesh->K2_AddRelativeRotation(setting.rot, false, res, true);
		ConnectorMesh->SetHiddenInGameSML(!setting.mesh, true);
		Connector->MaxCables = setting.maxCables;
		return;
	}
}

bool AFINNetworkAdapter::ShouldSave_Implementation() const {
	return true;
}

UFINNetworkAdapterReference::UFINNetworkAdapterReference() {}

UFINNetworkAdapterReference::~UFINNetworkAdapterReference() {}
