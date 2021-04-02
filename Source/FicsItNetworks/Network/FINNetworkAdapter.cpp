#include "FINNetworkAdapter.h"
#include "FGPowerConnectionComponent.h"
#include "FGItemPickup_Spawnable.h"
#include "FINNetworkCable.h"
#include "Components/SceneComponent.h"

TArray<TPair<UClass*, FFINAdapterSettings>> AFINNetworkAdapter::settings = TArray<TPair<UClass*, FFINAdapterSettings>>();

void AFINNetworkAdapter::RegistererAdapterSetting(UClass* clazz, FFINAdapterSettings newSettings) {
	clazz->AddToRoot();
	AFINNetworkAdapter::settings.Add(TPair<UClass*, FFINAdapterSettings>{clazz, newSettings});
}

void AFINNetworkAdapter::RegistererAdapterSetting(FString BPPath, FFINAdapterSettings newSettings) {
	UClass* clazz = LoadObject<UClass>(nullptr, *BPPath);
	RegistererAdapterSetting(clazz, newSettings);
}

void AFINNetworkAdapter::RegisterAdapterSettings() {
	// init adapter settings
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk1/Build_StorageContainerMk1.Build_StorageContainerMk1_C")), FFINAdapterSettings{FVector(290,0,400), FRotator(0,-90,0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk2/Build_StorageContainerMk2.Build_StorageContainerMk2_C")), FFINAdapterSettings{FVector(290,0,800), FRotator(0,-90,0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/IndustrialFluidContainer/Build_IndustrialTank.Build_IndustrialTank_C")), FFINAdapterSettings{FVector(0,540,1250), FRotator(0,0,0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/StorageTank/Build_PipeStorageTank.Build_PipeStorageTank_C")), FFINAdapterSettings{FVector(180,180,600), FRotator(0,-45,0), true, 2});
}

void AFINNetworkAdapter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINNetworkAdapter, Parent);
}

AFINNetworkAdapter::AFINNetworkAdapter() {
	bAlwaysRelevant = true;
	SetReplicates(true);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"Root");
	RootComponent->SetMobility(EComponentMobility::Type::Static);

	Connector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>(L"Connector");
	Connector->SetupAttachment(RootComponent);
	Connector->bOuterAsRedirect = false;
	
	ConnectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(L"StaticMesh");
	ConnectorMesh->SetHiddenInGame(true, true);
	ConnectorMesh->SetupAttachment(RootComponent);
	ConnectorMesh->SetMobility(EComponentMobility::Type::Movable);
	ConnectorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Connector->MaxCables = 1;
}

AFINNetworkAdapter::~AFINNetworkAdapter() {}

void AFINNetworkAdapter::BeginPlay() {
	Super::BeginPlay();

	UStaticMesh* networkAdapterMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/Mesh_Adapter.Mesh_Adapter"));
	ConnectorMesh->SetStaticMesh(networkAdapterMesh);

	if (!IsValid(Parent)) {
		for (AFINNetworkCable* cable : Connector->ConnectedCables) {
			TArray<FInventoryStack> refund;
			IFGDismantleInterface::Execute_GetDismantleRefund(cable, refund);
			float radius;
			FVector pos = IFGDismantleInterface::Execute_GetRefundSpawnLocationAndArea(cable, cable->GetActorLocation(), radius);
			TArray<class AFGItemPickup_Spawnable*> drops;
			AFGItemPickup_Spawnable::CreateItemDropsInCylinder(cable->GetWorld(), refund, pos, radius, {cable}, drops);
			cable->Destroy();
		}
		Destroy();
		return;
	}
	
	Connector->RedirectionObject = Parent;
	
	Attachment = NewObject<UFINNetworkAdapterReference>((Parent) ? Parent : nullptr);
	Attachment->Ref = this;
	Attachment->RegisterComponent();

	bool done = false;
	if (Parent->IsA<AFGBuildable>()) {
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
			done = true;
		}
	}

	if (!done) for (auto setting_entry : settings) {
		auto clazz = setting_entry.Key;
		auto setting = setting_entry.Value;
		if (!Parent->IsA(clazz)) continue;
		FVector pos = Parent->GetActorTransform().TransformPosition(Parent->K2_GetActorLocation());
		SetActorLocationAndRotation(pos, Parent->GetActorRotation());
		FHitResult res;
		ConnectorMesh->K2_AddRelativeRotation(setting.rot, false, res, true);
		ConnectorMesh->SetHiddenInGame(!setting.mesh, true);
		Connector->MaxCables = setting.maxCables;
		break;
	}

	ConnectorMesh->SetMobility(EComponentMobility::Type::Static);
}

bool AFINNetworkAdapter::ShouldSave_Implementation() const {
	return true;
}

bool AFINNetworkAdapter::NeedTransform_Implementation() {
	return true;
}

void AFINNetworkAdapter::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(Parent);
}

UFINNetworkAdapterReference::UFINNetworkAdapterReference() {}

UFINNetworkAdapterReference::~UFINNetworkAdapterReference() {}
