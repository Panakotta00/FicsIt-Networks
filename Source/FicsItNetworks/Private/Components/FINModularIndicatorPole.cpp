#include "Components/FINModularIndicatorPole.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Network/FINMCPAdvConnector.h"

#define BToS(b) b ? L"true" : L"false"

AFINModularIndicatorPole::AFINModularIndicatorPole() {
	
	Connector = CreateDefaultSubobject<UFINMCPAdvConnector>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Connector->SetIsReplicated(true);
	Connector->MaxCables = 2;

	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINModularIndicatorPole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFINModularIndicatorPole, Extension);
	DOREPLIFETIME(AFINModularIndicatorPole, Vertical);
}

void AFINModularIndicatorPole::OnConstruction(const FTransform& transform) {
	Parts.Empty();
	//UE_LOG(LogFicsItNetworks_DebugRoze, Log, TEXT("AFINModularIndicatorPole::OnConstruction(%d,%s)"), Extension, BToS(Vertical));
	Super::OnConstruction(transform);
	if(IsValid(Connector)) {
		Connector->SetMobility(EComponentMobility::Movable);
		if(Vertical) {
			ConnectorLocation = VerticalConnectorLocation;
			Connector->SetRelativeLocation(ConnectorLocation);
		}else {
			ConnectorLocation = HorizontalConnectorLocation;
			Connector->SetRelativeLocation(ConnectorLocation);
		}
		Connector->SetMobility(EComponentMobility::Static);
	}
	if(Vertical) {
		SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, VerticalBaseMesh, VerticalExtensionMesh, VerticalAttachmentMesh, ConnectorMesh, this, RootComponent, Parts,
			VerticalBaseOffset, VerticalExtensionOffset, VerticalExtensionMultiplier, VerticalAttachmentOffset,
			 VerticalConnectorMeshOffset, VerticalConnectorMeshRotation, VerticalConnectorMeshScale);
		FVector vt = VerticalExtensionMultiplier * Extension;
		ModuleConnectionPoint = VerticalModuleConnectionPointOffset + FVector(-vt.Y, vt.X, vt.Z);
	}else {
		SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, NormalBaseMesh, NormalExtensionMesh, NormalAttachmentMesh, ConnectorMesh, this, RootComponent, Parts,
			HorizontalBaseOffset, HorizontalExtensionOffset, HorizontalExtensionMultiplier, HorizontalAttachmentOffset,
			HorizontalConnectorMeshOffset, HorizontalConnectorMeshRotation, HorizontalConnectorMeshScale);
		ModuleConnectionPoint = HorizontalModuleConnectionPointOffset + HorizontalExtensionMultiplier * Extension;
	}

}

void AFINModularIndicatorPole::BeginPlay() {
	RerunConstructionScripts();
	Super::BeginPlay();
}

void AFINModularIndicatorPole::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (bHasChanged) {
		bHasChanged = false;
		ForceNetUpdate();
	}
}

bool AFINModularIndicatorPole::ShouldSave_Implementation() const {
	return true;
}

int32 AFINModularIndicatorPole::GetDismantleRefundReturnsMultiplier() const {
	return 1;
}

AActor* AFINModularIndicatorPole::netFunc_getNext() {
	if (!IsValid(ChildModule)) ChildModule = nullptr;
	return ChildModule;
}

void AFINModularIndicatorPole::GetChildDismantleActors_Implementation(TArray<AActor*>& out_ChildDismantleActors) const {
	Super::GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	if(IsValid(ChildModule)) {
		out_ChildDismantleActors.Add(ChildModule);
		ChildModule->GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	}
}

AActor* AFINModularIndicatorPole::netFunc_getModule(int Index) {
	int i = 0;
	AFINModularIndicatorPoleModule* Obj = ChildModule;
	while (i < Index) {
		if(!IsValid(Obj)) {
			return nullptr;
		}
		Obj = Obj->NextChild;
		i++;
	}
	if(IsValid(Obj)) {
		return Obj;
	}
	return nullptr;
}

void AFINModularIndicatorPole::SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int Extension, bool IsVertical,
									UStaticMesh* BaseMesh,
									UStaticMesh* ExtMesh,
									UStaticMesh* AtachMesh,
									UStaticMesh* ConnectorMesh,
									AActor* Parent, USceneComponent* Attach,
									TArray<UStaticMeshComponent*>& OutParts,
									FVector BO, FVector EO, FVector EM, FVector AO,
									FVector CMO, FRotator CMR, FVector CMS
									) {
	//UE_LOG(LogFicsItNetworks_DebugRoze, Log, TEXT("SpawnComponents(%d,%s)"), Extension, BToS(IsVertical));
	UStaticMeshComponent* BaseMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	BaseMeshComponent->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	BaseMeshComponent->SetRelativeLocation(BO);
	BaseMeshComponent->SetRelativeRotation(FRotator(0, 90, 0));
	BaseMeshComponent->RegisterComponent();
	BaseMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	BaseMeshComponent->SetStaticMesh(BaseMesh);
	BaseMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(BaseMeshComponent);

	for(int i = 0; i < Extension; i++) {
		UStaticMeshComponent* ExtMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
		ExtMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		ExtMeshComponent->SetRelativeLocation(EO + EM * i);
		ExtMeshComponent->SetRelativeRotation(FRotator(0,0,0));
		ExtMeshComponent->RegisterComponent();
		ExtMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		ExtMeshComponent->SetStaticMesh(ExtMesh);
		ExtMeshComponent->SetMobility(EComponentMobility::Static);
		OutParts.Add(ExtMeshComponent);
	}
	
	UStaticMeshComponent* AttachMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	AttachMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AttachMeshComponent->SetRelativeLocation(AO + EM * Extension);
	AttachMeshComponent->SetRelativeRotation(FRotator(0,0,0));
	AttachMeshComponent->RegisterComponent();
	AttachMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	AttachMeshComponent->SetStaticMesh(AtachMesh);
	AttachMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(AttachMeshComponent);

	
	UStaticMeshComponent* ConnectorMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	ConnectorMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ConnectorMeshComponent->SetRelativeLocation(CMO);
	ConnectorMeshComponent->SetRelativeRotation(CMR);
	ConnectorMeshComponent->SetRelativeScale3D(CMS);
	ConnectorMeshComponent->RegisterComponent();
	ConnectorMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	ConnectorMeshComponent->SetStaticMesh(ConnectorMesh);
	ConnectorMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(ConnectorMeshComponent);
}


