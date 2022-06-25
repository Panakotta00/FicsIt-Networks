#include "FINModularIndicatorPole.h"
#include "FINModularIndicatorPoleHolo.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FicsItNetworks/Network/FINMCPAdvConnector.h"
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
			ConnectorLocation = FVector(18.2365, 0.323992,-12.806);
			Connector->SetRelativeLocation(ConnectorLocation);
		}else {
			ConnectorLocation = FVector(7.0538,-4.2,15.7995);
			Connector->SetRelativeLocation(ConnectorLocation);
		}
		Connector->SetMobility(EComponentMobility::Static);
	}
	if(Vertical) {
		SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, VerticalBaseMesh, VerticalExtensionMesh, VerticalAttachmentMesh, ConnectorMesh, this, RootComponent, Parts );
		ModuleConnectionPoint = FVector(15.9591 + 40.8858 + (float)(Extension) * 10.6,0, 61.6469);
	}else {
		SpawnComponents(UStaticMeshComponent::StaticClass(), Extension, Vertical, NormalBaseMesh, NormalExtensionMesh, NormalAttachmentMesh, ConnectorMesh, this, RootComponent, Parts);
		ModuleConnectionPoint = FVector(0,0, 32.3555 + 6.66872 + (float)(Extension) * 26.6992);
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

AFINModularIndicatorPoleModule* AFINModularIndicatorPole::netFunc_getNext() {
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

AFINModularIndicatorPoleModule* AFINModularIndicatorPole::netFunc_getModule(int Index) {
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
									TArray<UStaticMeshComponent*>& OutParts) {
	//UE_LOG(LogFicsItNetworks_DebugRoze, Log, TEXT("SpawnComponents(%d,%s)"), Extension, BToS(IsVertical));
	UStaticMeshComponent* BaseMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	BaseMeshComponent->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	BaseMeshComponent->SetRelativeLocation(FVector(0,0, 0));
	if(IsVertical) {
		BaseMeshComponent->SetRelativeRotation(FRotator(0, 90, 0));
	}else {
		BaseMeshComponent->SetRelativeRotation(FRotator(0,90,0)); // FRotator(0,270,90)
	}
	BaseMeshComponent->RegisterComponent();
	BaseMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	BaseMeshComponent->SetStaticMesh(BaseMesh);
	BaseMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(BaseMeshComponent);

	for(int i = 0; i < Extension; i++) {
		UStaticMeshComponent* ExtMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
		ExtMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		if(IsVertical) {
			ExtMeshComponent->SetRelativeLocation(FVector(0, -17.0591 - (float)(i) * 10.6, 1.6));
			ExtMeshComponent->SetRelativeRotation(FRotator(0,0,0));
		}else {
			ExtMeshComponent->SetRelativeLocation(FVector(0,0, 32.3555 + (float)(i) * 26.6992));
			ExtMeshComponent->SetRelativeRotation(FRotator(0,0,0));
		}
		ExtMeshComponent->RegisterComponent();
		ExtMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		ExtMeshComponent->SetStaticMesh(ExtMesh);
		ExtMeshComponent->SetMobility(EComponentMobility::Static);
		OutParts.Add(ExtMeshComponent);
	}
	
	UStaticMeshComponent* AttachMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	AttachMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	if(IsVertical) {
		AttachMeshComponent->SetRelativeLocation(FVector(0, -15.9591 - (float)(Extension) * 10.6,0));
		AttachMeshComponent->SetRelativeRotation(FRotator(0,0,0));
	}else {
		AttachMeshComponent->SetRelativeLocation(FVector(0,0, 32.3555 + (float)(Extension) * 26.6992));
		AttachMeshComponent->SetRelativeRotation(FRotator(0,0,0));
	}
	AttachMeshComponent->RegisterComponent();
	AttachMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	AttachMeshComponent->SetStaticMesh(AtachMesh);
	AttachMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(AttachMeshComponent);

	
	UStaticMeshComponent* ConnectorMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	ConnectorMeshComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	if(IsVertical) {
		ConnectorMeshComponent->SetRelativeLocation(FVector(-8.1,-13.5,-13));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, -90));
		ConnectorMeshComponent->SetRelativeScale3D(FVector(2.4, 2.4, 1.8));
	}else {
		ConnectorMeshComponent->SetRelativeLocation(FVector(-8.4,-2.9,15.8));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, -90));
		ConnectorMeshComponent->SetRelativeScale3D(FVector(1.2, 1.2, 1.6));
	}
	ConnectorMeshComponent->RegisterComponent();
	ConnectorMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	ConnectorMeshComponent->SetStaticMesh(ConnectorMesh);
	ConnectorMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(ConnectorMeshComponent);
}


