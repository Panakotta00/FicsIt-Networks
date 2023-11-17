#include "Network/FINNetworkAdapter.h"
#include "Network/FiNNetworkCable.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGPowerConnectionComponent.h"
#include "FGItemPickup_Spawnable.h"
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
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/IndustrialFluidContainer/Build_IndustrialTank.Build_IndustrialTank_C")), FFINAdapterSettings{FVector(0,540,1250), FRotator(0,-45,0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/StorageTank/Build_PipeStorageTank.Build_PipeStorageTank_C")), FFINAdapterSettings{FVector(180,180,600), FRotator(0,-90,0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/PipeValve/Build_Valve.Build_Valve_C")), FFINAdapterSettings{FVector(0,0,0), FRotator(0,0,0), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/Train/Signal/Build_RailroadBlockSignal.Build_RailroadBlockSignal_C")), FFINAdapterSettings{FVector(0, 470, 936), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/Train/Signal/Build_RailroadPathSignal.Build_RailroadPathSignal_C")), FFINAdapterSettings{FVector(0, 470, 936), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/Train/Signal/Build_RailroadPathSignal.Build_RailroadPathSignal_C")), FFINAdapterSettings{FVector(0, 470, 936), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Huge.Build_StandaloneWidgetSign_Huge_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Large.Build_StandaloneWidgetSign_Large_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Medium.Build_StandaloneWidgetSign_Medium_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Portrait.Build_StandaloneWidgetSign_Portrait_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Small.Build_StandaloneWidgetSign_Small_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_SmallVeryWide.Build_StandaloneWidgetSign_SmallVeryWide_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_SmallWide.Build_StandaloneWidgetSign_SmallWide_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Square.Build_StandaloneWidgetSign_Square_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Square_Small.Build_StandaloneWidgetSign_Square_Small_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/SignDigital/Build_StandaloneWidgetSign_Square_Tiny.Build_StandaloneWidgetSign_Square_Tiny_C")), FFINAdapterSettings{FVector(0, -20, 0), FRotator(), false, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Building/Doors/Build_Gate_Automated_8x4.Build_Gate_Automated_8x4_C")), FFINAdapterSettings{FVector(75, 300, 350), FRotator(0, -90, 0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Building/Doors/BUILD_SingleDoor_Base_01.BUILD_SingleDoor_Base_01_C")), FFINAdapterSettings{FVector(75, 0, 350), FRotator(0, -90, 0), true, 2});
	RegistererAdapterSetting(FString(TEXT("/Game/FactoryGame/Buildable/Factory/Train/SwitchControl/Build_RailroadSwitchControl.Build_RailroadSwitchControl_C")), FFINAdapterSettings{FVector(-200, 400, 150), FRotator(), false, 1});
}

bool AFINNetworkAdapter::FindConnection(AActor* Actor, FVector HitLocation, FTransform& OutTransform, bool& OutMesh, int& OutMaxCables) {
	if (Actor->IsA<AFGBuildable>()) {
		// Try to find fitting component
		TArray<UActorComponent*> Components;
		Actor->GetComponents(USceneComponent::StaticClass(), Components);
		float Distance = -1.0f;
		USceneComponent* FoundComponent = nullptr;
		for (UActorComponent* Component : Components) {
			bool bShouldSnap = false;
			bool bMesh = false;

			UFGPowerConnectionComponent* Power = Cast<UFGPowerConnectionComponent>(Component);
			if (Power) {
				bShouldSnap = !(Power->GetMaxNumConnections() < 1 || Power->IsHidden());
			} else if (Cast<USceneComponent>(Component)->GetName().EndsWith("FINConnector")) {
				bShouldSnap = true;
				bMesh = Cast<USceneComponent>(Component)->GetName().EndsWith("Visible_FINConnector");
			}
			
			if (bShouldSnap) {
				float ComponentDistance = (Cast<USceneComponent>(Component)->GetComponentToWorld().GetTranslation() - HitLocation).Size();
				if (Distance < 0.0f || Distance > ComponentDistance) {
					Distance = ComponentDistance;
					FoundComponent = Cast<USceneComponent>(Component);
					OutMesh = bMesh;
				}
			}
		}
		if (FoundComponent) {
			OutTransform = FoundComponent->GetComponentTransform();
			return true;
		}

		// find pre defined adapter setting
		for (const TPair<UClass*, FFINAdapterSettings>& entry : AFINNetworkAdapter::settings) {
			const FFINAdapterSettings& setting = entry.Value;
			UClass* clazz = entry.Key;

			if (Actor->IsA(clazz)) {
				FVector translation = Actor->GetTransform().TransformPosition(setting.loc);
				FQuat rotation = Actor->GetTransform().TransformRotation(setting.rot.Quaternion());
				OutTransform = FTransform(rotation, translation);
				OutMesh = setting.mesh;
				OutMaxCables = setting.maxCables;
				return true;
			}
		}
	}
	
	return false;
}

void AFINNetworkAdapter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINNetworkAdapter, Parent);
	DOREPLIFETIME(AFINNetworkAdapter, Connector);
}

AFINNetworkAdapter::AFINNetworkAdapter() {
	bAlwaysRelevant = true;
	SetReplicates(true);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"Root");
	RootComponent->SetMobility(EComponentMobility::Type::Static);

	Connector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>(L"Connector");
	Connector->SetupAttachment(RootComponent);
	Connector->bOuterAsRedirect = false;
	Connector->SetIsReplicated(true);
	
	ConnectorMesh = CreateDefaultSubobject<UFGColoredInstanceMeshProxy>(L"StaticMesh");
	ConnectorMesh->SetupAttachment(RootComponent);
	ConnectorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Connector->MaxCables = 1;
}

AFINNetworkAdapter::~AFINNetworkAdapter() {}

void AFINNetworkAdapter::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	//ConnectorMesh->SetHiddenInGame(true, true);
	//ConnectorMesh->SetVisibility(false);
}

void AFINNetworkAdapter::BeginPlay() {
	Super::BeginPlay();

	if (!IsValid(Parent)) {
		for (AFINNetworkCable* cable : Connector->ConnectedCables) {
			TArray<FInventoryStack> refund;
			IFGDismantleInterface::Execute_GetDismantleRefund(cable, refund, false);
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

	FTransform ConnectorTransform;
	bool bMesh;
	int MaxCables;
	if (FindConnection(Parent, GetActorLocation(), ConnectorTransform, bMesh, MaxCables)) {
		SetActorTransform(ConnectorTransform);
		Connector->MaxCables = MaxCables;
		ConnectorMesh->SetHiddenInGame(!bMesh, true);
		ConnectorMesh->SetInstanced(false);
		ConnectorMesh->SetInstanced(true);
		ConnectorMesh->SetVisibility(bMesh);
	}
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
