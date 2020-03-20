#include "FINNetworkAdapter.h"

#include "FGPowerConnectionComponent.h"

#include "Components/SceneComponent.h"

std::vector<std::pair<UClass*, FFINAdapterSettings>> AFINNetworkAdapter::settings = std::vector<std::pair<UClass*, FFINAdapterSettings>>();

AFINNetworkAdapter::AFINNetworkAdapter() {
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> networkAdapterMesh(TEXT("StaticMesh'/Game/FicsIt-Networks/ComputerNetwork/Mesh_Adapter.Mesh_Adapter"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(L"Root");

	Connector = CreateDefaultSubobject<UFINNetworkConnector>(L"Connector");
	Connector->SetupAttachment(RootComponent);
	
	ConnectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(L"StaticMesh");
	ConnectorMesh->SetHiddenInGame(true, true);
	ConnectorMesh->SetupAttachment(RootComponent, FName());
	//ConnectorMesh->SetStaticMesh(networkAdapterMesh.Object);

	Connector->MaxCables = 1;
}

AFINNetworkAdapter::~AFINNetworkAdapter() {}

void AFINNetworkAdapter::BeginPlay() {
	Super::BeginPlay();

	Connector->Merged.Add(Parent);

	Attachment = NewObject<UFINNetworkAdapterReference>(Parent);
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
		SetActorLocationAndRotation(Parent->K2_GetActorLocation(), Parent->GetActorRotation());
		AddActorLocalOffset(setting.loc);
		ConnectorMesh->AddRelativeRotation(setting.rot);
		ConnectorMesh->SetHiddenInGame(!setting.mesh, true);
		Connector->MaxCables = setting.maxCables;
		return;
	}
}

bool AFINNetworkAdapter::ShouldSave_Implementation() const {
	return true;
}

UFINNetworkAdapterReference::UFINNetworkAdapterReference() {}

UFINNetworkAdapterReference::~UFINNetworkAdapterReference() {}

// TODO: Add Settings
/*
// init adapter settings
ANetworkAdapter::addSetting(L"Buildable/Factory/StorageContainerMk1", L"Build_StorageContainerMk1", {FVector{290,0,400}, SDK::FRotator{0,-90,0}, true, 1});
ANetworkAdapter::settings.push_back({SDK::AFGBuildableStorage::StaticClass(), AdapterSettings{FVector{0,0,0}, SDK::FRotator{0,0,0}, true, 1}});
*/